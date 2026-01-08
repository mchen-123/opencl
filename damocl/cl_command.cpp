#include <algorithm>
#include "dfcl_cl.hpp"

std::atomic<int> queue_c{0};

extern "C" CL_API_ENTRY cl_command_queue CL_API_CALL
clCreateCommandQueue(cl_context context, 
                    cl_device_id device,
                    cl_command_queue_properties properties,
                    cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    cl_int errcode = CL_SUCCESS;
    cl_uint i = 0;
    cl_bool found = CL_FALSE;

    if (!is_valid(context)) {
        DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_CONTEXT, errcode_ret, nullptr);
    }
    if (!is_valid(device) || (device->available != CL_TRUE)) {
        DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_DEVICE, errcode_ret, nullptr);
    }
    
    /* validate flags */
    cl_command_queue_properties all_properties
        = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE
        | CL_QUEUE_ON_DEVICE | CL_QUEUE_ON_DEVICE_DEFAULT | CL_QUEUE_HIDDEN;

    if ((properties & (~all_properties))) {
        DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_VALUE, errcode_ret, nullptr);
    }
    cl_command_queue_properties supported_device_props;
    if (properties & (CL_QUEUE_ON_DEVICE | CL_QUEUE_ON_DEVICE_DEFAULT))
        supported_device_props = device->on_dev_queue_props;
    else
        supported_device_props = device->on_host_queue_props | CL_QUEUE_HIDDEN;

    if ((properties & supported_device_props) != properties) {
        DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_QUEUE_PROPERTIES, errcode_ret, nullptr);
    }

    for (i = 0; i < context->num_devices; ++i) {
        if (context->devices[i] == device) {
            found = true;
        }
    }
    if (!found) {
        DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_DEVICE, errcode_ret, nullptr);
    }

    cl_command_queue command_queue = new _cl_command_queue();
    if (command_queue == nullptr) {
        DFCL_SET_FUNCTION_VALUE_RETURN(CL_OUT_OF_HOST_MEMORY, errcode_ret, nullptr);
    }

    CL_INIT_OBJECT(command_queue, context);

    command_queue->context = context;
    command_queue->device = device;
    command_queue->properties = properties;
    
    /* hidden queues don't retain the context. */
    if ((properties & CL_QUEUE_HIDDEN) == 0) {
        dfcl_retain_object(context);
        DfclObject::LockGuard lock(context);
        context->command_queues.push_back(command_queue);
    }

    errcode = CL_SUCCESS;
    if (device->ops->init_queue)
        errcode = device->ops->init_queue(device, command_queue);

    DFCL_ATOMIC_INC(queue_c);

    if (errcode_ret != nullptr)
        *errcode_ret = errcode;

    DFCL_MSG_INFO("Create Commandqueue: " << command_queue << ", RefCount: " << command_queue->getRefCount() << " \n");

    return command_queue;
}

// extern "C" CL_API_ENTRY cl_command_queue CL_API_CALL
// clCreateCommandQueueWithProperties(cl_context context, 
//                                       cl_device_id device,
//                                       [[maybe_unused]] const cl_queue_properties* properties,
//                                       cl_int* errcode_ret) {
//     cl_int result = CL_SUCCESS;
//     cl_command_queue cl_queue = nullptr;
//     DFStream stream = nullptr;
                           
//     if (!is_valid(context)) {
//         DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_CONTEXT, errcode_ret, cl_queue)
//     }
//     if (!is_valid(device)) {
//         DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_DEVICE, errcode_ret, cl_queue)
//     }

//     auto ctx = as_internal(context);
//     auto dev = as_internal(device);
//     dfCtxPushCurrent(ctx->getContexts()[dev->id()]);
//     if (dfStreamCreate(&stream, 0) != DF_SUCCESS) {
//         DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_VALUE, errcode_ret, cl_queue)
//     }
//     if (errcode_ret)
//         *errcode_ret = result;
    
//     auto oclQueue = DF_NEW(OpenclCommandqueue(stream));
//     if (!oclQueue) {
//         dfStreamDestroy(stream);
//         DFCL_SET_FUNCTION_VALUE_RETURN(CL_OUT_OF_HOST_MEMORY, errcode_ret, cl_queue)
//     }
//     DFCL_SET_FUNCTION_VALUE_RETURN(CL_SUCCESS, errcode_ret, as_cl(oclQueue))
// }

extern "C" CL_API_ENTRY cl_int CL_API_CALL
clFlush(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0 
{
    if (!is_valid(command_queue)) {
        return CL_INVALID_COMMAND_QUEUE;
    }

    if (command_queue->device->available == CL_FALSE) {
        return CL_DEVICE_NOT_AVAILABLE;
    }

    if (command_queue->device->ops->flush)
        command_queue->device->ops->flush(command_queue->device, command_queue);
    return CL_SUCCESS;
}

extern "C" CL_API_ENTRY cl_int CL_API_CALL
clReleaseCommandQueue(cl_command_queue command_queue) {
    if (!is_valid(command_queue)) {
        return CL_INVALID_COMMAND_QUEUE;
    }

    int new_refcount = dfcl_release_object(command_queue);
    DFCL_MSG_INFO("Release Command Queue= " << command_queue << ", new_refcount =" << new_refcount << " \n");

    if (new_refcount == 0) {

        cl_context context = command_queue->context;
        cl_device_id device = command_queue->device;

        DFCL_ATOMIC_DEC(queue_c);
        if ((command_queue->properties & CL_QUEUE_HIDDEN) == 0) {
            DfclObject::LockGuard lock(context);
            auto& queues = context->command_queues;
            auto it = std::find(queues.begin(), queues.end(), command_queue);
            if (it != queues.end()) {
                queues.erase(it);
            } else {
                DFCL_MSG_INFO("Command Queue not found in context's queue list.");
            }

            dfcl_release_object(context);
        }

        assert(command_queue->command_count == 0);

        if (command_queue->device->ops->free_queue
            && (command_queue->device->available)== CL_TRUE)
        command_queue->device->ops->free_queue(device, command_queue);
        
        DFCL_MEM_FREE(command_queue);
    }

    return CL_SUCCESS;
}

