#include "cl_context.hpp"
#include "cl_device.hpp"

extern "C" CL_API_ENTRY cl_context  CL_API_CALL
clCreateContext([[maybe_unused]] const cl_context_properties* properties,
                           cl_uint num_devices,
                           const cl_device_id* devices,
                           [[maybe_unused]] void(CL_CALLBACK* pfn_notify)(const char *, const void *, size_t, void *),
                           [[maybe_unused]] void* user_data,
                           cl_int* errcode_ret) {
    cl_int result = CL_SUCCESS;

    int platform_device_count = 0;
    if (dfDeviceGetCount(&platform_device_count) != DF_SUCCESS || platform_device_count == 0) {
        CL_SET_FUNCTION_VALUE_RETURN(CL_DEVICE_NOT_AVAILABLE, errcode_ret, nullptr)
    }

    if (!devices || num_devices == 0 || num_devices > static_cast<cl_uint>(platform_device_count)) {
        CL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_VALUE, errcode_ret, nullptr)
    }

    DFContext* pCtx = DF_NEW_ARRAY(DFContext, num_devices);
    if (!pCtx) {
        CL_SET_FUNCTION_VALUE_RETURN(CL_OUT_OF_HOST_MEMORY, errcode_ret, nullptr)
    }
    memset(pCtx, 0, sizeof(DFContext) * num_devices);

    bool createFailed = false;
    for(cl_uint index = 0; index < num_devices; index++) {
        auto* oclDev = as_runtime(devices[index]);
        if (!oclDev || !oclDev->get()) {
            createFailed = true;
            break;
        }
        if(dfCtxCreate(&pCtx[index], 0, oclDev->get()) != DF_SUCCESS) {
            createFailed = true;
            break;
        }
    }
    if (createFailed) {
        for (cl_uint i = 0; i < num_devices; ++i) {
            if (pCtx[i] != nullptr) {
                dfCtxDestroy(pCtx[i]);
            }
        }
        DF_DELETE_ARRAY(pCtx);
        CL_SET_FUNCTION_VALUE_RETURN(CL_OUT_OF_RESOURCES, errcode_ret, nullptr)
    }

    auto* oclCtx = DF_NEW(OpenclContext(pCtx, num_devices, platform_device_count));
    if (!oclCtx) {
        for (cl_uint i = 0; i < num_devices; ++i) {
            dfCtxDestroy(pCtx[i]);
        }
        DF_DELETE_ARRAY(pCtx);
        CL_SET_FUNCTION_VALUE_RETURN(CL_OUT_OF_HOST_MEMORY, errcode_ret, nullptr)
    }

    CL_SET_FUNCTION_VALUE_RETURN(CL_SUCCESS, errcode_ret, as_cl(oclCtx))
}

extern "C" CL_API_ENTRY cl_int CL_API_CALL
clReleaseContext(cl_context context) {
    if (!is_valid(context)) {
        return CL_INVALID_CONTEXT;
    }

    auto* oclCtx = as_runtime(context);
    if (!oclCtx) {
        return CL_INVALID_CONTEXT;
    }
    oclCtx->release();
    return CL_SUCCESS;
}
