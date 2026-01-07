#include <mutex>

#include "devices.hpp"
#include "dfcl_cl.hpp"

extern cl_platform_id damoPlatform;
extern int dfcl_offline_compile;

uint cl_context_count = 0;
static std::mutex dfcl_context_handling_lock;

int
context_set_properties(cl_context context, 
                       const cl_context_properties *properties) 
{
    if (properties == nullptr) {
        context->properties = nullptr;
        context->num_properties = 0;
        return 0;
    }
    // todo: properties are not supported yet
    return 0;
}

// todo: fill in the context structure
int
dfcl_setup_context([[maybe_unused]] cl_context context) {
    return CL_SUCCESS;
}

extern "C" CL_API_ENTRY cl_context  CL_API_CALL
clCreateContext(const cl_context_properties* properties,
                    cl_uint num_devices,
                    const cl_device_id* devices,
                    void(CL_CALLBACK* pfn_notify)(const char *, const void *, size_t, void *),
                    void* user_data,
                    cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_0 
{
    cl_int errcode = CL_SUCCESS;
    cl_uint i = 0;
    cl_context context = nullptr;

    if (devices == NULL || num_devices == 0) {
        DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_VALUE, errcode_ret, nullptr);
    }
    if (pfn_notify == nullptr && user_data != nullptr) {
        DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_VALUE, errcode_ret, nullptr);
    }
    
    {
        std::lock_guard<std::mutex> lock(dfcl_context_handling_lock);
        errcode = dfcl_init_devices(damoPlatform);
        /* clCreateContext cannot return CL_DEVICE_NOT_FOUND, which is what
        * dfcl_init_devices() returns if no devices could be probed. Hence,
        * remap this error to CL_INVALID_DEVICE. Note that this particular
        * situation should never arise, since an application should issue
        * clGetDeviceIDs before clCreateContext, and we would have returned
        * CL_DEVICE_NOT_FOUND already from clGetDeviceIDs. Still, no reason
        * not to handle it.
        */
        if (errcode == CL_DEVICE_NOT_FOUND) {
            errcode = CL_INVALID_DEVICE;
        }
        if (errcode != CL_SUCCESS) {
            DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_DEVICE, errcode_ret, nullptr);
        }
        
        for (i = 0; i < num_devices; i++) {
            if (devices[i] == nullptr) {
               DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_DEVICE, errcode_ret, nullptr);
            }
        }

        context = (cl_context)calloc(1, sizeof(_cl_context));
        if (context == nullptr) {
            DFCL_SET_FUNCTION_VALUE_RETURN(CL_OUT_OF_HOST_MEMORY, errcode_ret, nullptr);
        }    
        CL_INIT_OBJECT(context, devices[0]);

        // todo: 1. raw ptrs create
        errcode = context_set_properties(context, properties);
        if (errcode != CL_SUCCESS) {
            DFCL_MEM_FREE(context);
            DFCL_SET_FUNCTION_VALUE_RETURN(errcode, errcode_ret, nullptr);
        }

        context->create_devices = (cl_device_id *)calloc(num_devices, sizeof(cl_device_id));
        if (context->create_devices == nullptr) {
            DFCL_MEM_FREE(context->properties);
            DFCL_MEM_FREE(context);
            DFCL_SET_FUNCTION_VALUE_RETURN(errcode, errcode_ret, nullptr);
        }  
        memcpy(context->create_devices, devices, num_devices * sizeof(cl_device_id));
        context->num_create_devices = num_devices;

        // todo: 2. transform subDevice to parent device.
        context->devices = context->create_devices;
        context->num_devices = num_devices;

        context->default_queues = (cl_command_queue *)calloc(num_devices, sizeof(cl_command_queue));
        if (context->default_queues == nullptr) {
            DFCL_MEM_FREE(context->create_devices);
            DFCL_MEM_FREE(context->properties);
            DFCL_MEM_FREE(context);
            DFCL_SET_FUNCTION_VALUE_RETURN(CL_OUT_OF_HOST_MEMORY, errcode_ret, nullptr);
        }

        for (i = 0; i < num_devices; i++) {
            cl_device_id dev = context->devices[i];
            if (!dfcl_offline_compile && (dev->available == CL_FALSE)) {
                DFCL_MEM_FREE(context->create_devices);
                DFCL_MEM_FREE(context->properties);
                DFCL_MEM_FREE(context->default_queues);
                DFCL_MEM_FREE(context);
                DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_DEVICE, errcode_ret, nullptr);
            }
        }

        if (!dfcl_offline_compile) {
            errcode = dfcl_setup_context(context);
            if (errcode != CL_SUCCESS) {
                DFCL_MEM_FREE(context->create_devices);
                DFCL_MEM_FREE(context->properties);
                DFCL_MEM_FREE(context->default_queues);
                DFCL_MEM_FREE(context);
                DFCL_SET_FUNCTION_VALUE_RETURN(errcode, errcode_ret, nullptr);
            }
        }
        for (i = 0; i < context->num_create_devices; i++) {
            if (devices[i] == nullptr || devices[i]->available != CL_TRUE || devices[i]->parent_device == nullptr)
                continue;
            dfcl_retain_object(devices[i]);
        }
    }
    
    if (errcode_ret) {
        *errcode_ret = CL_SUCCESS;
    }

    cl_context_count += 1;
    return context;
}

// extern "C" CL_API_ENTRY cl_int CL_API_CALL
// clGetContextInfo(cl_context context, cl_context_info param_name, size_t param_value_size,
//                  void* param_value, size_t* param_value_size_ret) {
//     if (!is_valid(context)) {
//         return CL_INVALID_CONTEXT;
//     }

//     switch(param_name) {
//         case CL_CONTEXT_PROPERTIES: {
//             auto* oclCtx = as_internal(context);
//             size_t valueSize = oclCtx->getPropertiesSize();

//             if (param_value != nullptr && param_value_size < valueSize) {
//                 return CL_INVALID_VALUE;
//             }

//             *param_value_size_ret = valueSize;
//             if (param_value != nullptr && valueSize != 0) {
//                 ::memcpy(param_value, oclCtx->properties(), valueSize); 
//             }
//             return CL_SUCCESS;
//         }
//         default: 
//             return CL_INVALID_VALUE;
//     }
//     return CL_SUCCESS;
// }


extern "C" CL_API_ENTRY cl_int CL_API_CALL
clReleaseContext(cl_context context) CL_API_SUFFIX__VERSION_1_0
{
    if (!is_valid(context)) {
        return CL_INVALID_CONTEXT;
    }

    std::lock_guard<std::mutex> lock(dfcl_context_handling_lock);
    cl_uint i = 0;
    int new_refcount = dfcl_release_object(context);
    DFCL_MSG_INFO("release context= " << context << ", new_refcount =" << new_refcount << " \n");

    if (new_refcount == 0) {
        for (i = 0; i < context->num_devices; i++) {
            // cl_device_id dev = context->devices[i];
            if (context->default_queues && context->default_queues[i]) {
                // todo: release default queue
            }
        }

        for (i = 0; i < context->num_create_devices; i++) {
            // todo: +releaseDevice(context->create_devices[i]);
        }

        DFCL_MEM_FREE(context->create_devices);
        DFCL_MEM_FREE(context->default_queues);
        // DFCL_MEM_FREE(context->devices);
        DFCL_MEM_FREE(context->properties);

        DFCL_MEM_FREE(context);
        --cl_context_count;
    }

    return CL_SUCCESS;
}
