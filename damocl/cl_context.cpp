#include "cl_context.hpp"
#include "cl_device.hpp"

extern cl_platform_id damoPlatform;

extern "C" CL_API_ENTRY cl_context  CL_API_CALL
clCreateContext(const cl_context_properties* properties,
                           cl_uint num_devices,
                           const cl_device_id* devices,
                           [[maybe_unused]] void(CL_CALLBACK* pfn_notify)(const char *, const void *, size_t, void *),
                           [[maybe_unused]] void* user_data,
                           cl_int* errcode_ret) {
    cl_int result = CL_SUCCESS;
    cl_platform_id platform = nullptr;
    size_t count = 0;

    std::unique_ptr<cl_context_properties[]> props;
    if (is_valid(properties)) {
        for (const cl_context_properties* p = properties; *p != 0; p += 2) {
            if (p[0] == CL_CONTEXT_PLATFORM) {
                platform = reinterpret_cast<cl_platform_id>(p[1]);
                if (platform != nullptr && platform != damoPlatform) {
                    CL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_VALUE, errcode_ret, nullptr)
                }
            } else {
                CL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_PROPERTY, errcode_ret, nullptr)
            }
            ++count;
        }
        props.reset(new cl_context_properties[2 * count + 1]);
        std::copy(properties, properties + 2 * count + 1, props.get());
    }

    int platform_device_count = 0;
    if (dfDeviceGetCount(&platform_device_count) != DF_SUCCESS || platform_device_count <= 0) {
        CL_SET_FUNCTION_VALUE_RETURN(CL_DEVICE_NOT_AVAILABLE, errcode_ret, nullptr)
    }

    if (!devices || num_devices == 0 || num_devices > static_cast<cl_uint>(platform_device_count)) {
        CL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_VALUE, errcode_ret, nullptr)
    }

    auto pCtx= std::make_unique<DFContext[]>(num_devices);
    std::fill(pCtx.get(), pCtx.get() + num_devices, nullptr);

    for(cl_uint index = 0; index < num_devices; index++) {
        auto* oclDev = as_internal(devices[index]);
        if (!oclDev || !oclDev->get()) {
            CL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_DEVICE, errcode_ret, nullptr)
        }

        if(dfCtxCreate(&pCtx[index], 0, oclDev->get()) != DF_SUCCESS) {
            for (cl_uint j = 0; j < index; ++j) {
                if (pCtx[j]) dfCtxDestroy(pCtx[j]);
            }
            CL_SET_FUNCTION_VALUE_RETURN(CL_OUT_OF_RESOURCES, errcode_ret, nullptr)
        }
    }

    auto* oclCtx = DF_NEW(OpenclContext(pCtx.release(), num_devices, platform_device_count));
    if (!oclCtx) {
        for (cl_uint i = 0; i < num_devices; ++i) {
            dfCtxDestroy(pCtx[i]);
        }
        CL_SET_FUNCTION_VALUE_RETURN(CL_OUT_OF_HOST_MEMORY, errcode_ret, nullptr)
    }

    oclCtx->setProperties(props.release());
    oclCtx->setPropertiesSize((2 * count + 1) * sizeof(cl_context_properties));
    CL_SET_FUNCTION_VALUE_RETURN(CL_SUCCESS, errcode_ret, as_cl(oclCtx))
}

// (context, CL_CONTEXT_PROPERTIES,
//             //                        sizeof(cl_context_properties) * 3,
//             //                        properties, &returned_size)
extern "C" CL_API_ENTRY cl_int CL_API_CALL
clGetContextInfo(cl_context context, cl_context_info param_name, size_t param_value_size,
                 void* param_value, size_t* param_value_size_ret) {
    if (!is_valid(context)) {
        return CL_INVALID_CONTEXT;
    }

    switch(param_name) {
        case CL_CONTEXT_PROPERTIES: {
            auto* oclCtx = as_internal(context);
            size_t valueSize = oclCtx->getPropertiesSize();

            if (param_value != nullptr && param_value_size < valueSize) {
                return CL_INVALID_VALUE;
            }

            *param_value_size_ret = valueSize;
            if (param_value != nullptr && valueSize != 0) {
                ::memcpy(param_value, oclCtx->properties(), valueSize); 
            }
            return CL_SUCCESS;
        }
        default: 
            return CL_INVALID_VALUE;
    }
    return CL_SUCCESS;
}


extern "C" CL_API_ENTRY cl_int CL_API_CALL
clReleaseContext(cl_context context) {
    if (!is_valid(context)) {
        return CL_INVALID_CONTEXT;
    }

    auto* oclCtx = as_internal(context);
    if (!oclCtx) {
        return CL_INVALID_CONTEXT;
    }
    oclCtx->release();
    return CL_SUCCESS;
}
