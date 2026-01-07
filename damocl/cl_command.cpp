#include "cl_command.hpp"
#include "cl_context.hpp"
#include "cl_device.hpp"

extern "C" CL_API_ENTRY cl_command_queue CL_API_CALL
clCreateCommandQueueWithProperties(cl_context context, 
                                      cl_device_id device,
                                      [[maybe_unused]] const cl_queue_properties* properties,
                                      cl_int* errcode_ret) {
    cl_int result = CL_SUCCESS;
    cl_command_queue cl_queue = nullptr;
    DFStream stream = nullptr;
                           
    if (!is_valid(context)) {
        DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_CONTEXT, errcode_ret, cl_queue)
    }
    if (!is_valid(device)) {
        DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_DEVICE, errcode_ret, cl_queue)
    }

    auto ctx = as_internal(context);
    auto dev = as_internal(device);
    dfCtxPushCurrent(ctx->getContexts()[dev->id()]);
    if (dfStreamCreate(&stream, 0) != DF_SUCCESS) {
        DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_VALUE, errcode_ret, cl_queue)
    }
    if (errcode_ret)
        *errcode_ret = result;
    
    auto oclQueue = DF_NEW(OpenclCommandqueue(stream));
    if (!oclQueue) {
        dfStreamDestroy(stream);
        DFCL_SET_FUNCTION_VALUE_RETURN(CL_OUT_OF_HOST_MEMORY, errcode_ret, cl_queue)
    }
    DFCL_SET_FUNCTION_VALUE_RETURN(CL_SUCCESS, errcode_ret, as_cl(oclQueue))
}

extern "C" CL_API_ENTRY cl_int CL_API_CALL
clFinish(cl_command_queue command_queue) {
    if (!is_valid(command_queue)) {
        return CL_INVALID_COMMAND_QUEUE;
    }
    auto oclStream = as_internal(command_queue);
    if (dfStreamSynchronize(oclStream->get()) != DF_SUCCESS) {
        return CL_INVALID_VALUE;
    }
    return CL_SUCCESS;
}

extern "C" CL_API_ENTRY cl_int CL_API_CALL
clReleaseCommandQueue(cl_command_queue command_queue) {
    if (!is_valid(command_queue)) {
        return CL_INVALID_COMMAND_QUEUE;
    }
    auto oclQueue = as_internal(command_queue);
    oclQueue->release();
    return CL_SUCCESS;
}

