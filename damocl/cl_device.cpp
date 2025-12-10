#include "cl_device.hpp"
#include <sstream>

#ifndef CL_USE_DEPRECATED_OPENCL_1_0_APIS
#define CL_USE_DEPRECATED_OPENCL_1_0_APIS
#endif

#ifndef CL_USE_DEPRECATED_OPENCL_1_1_APIS
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#endif

static cl_bool g_initialized = CL_FALSE;
cl_platform_id damoPlatform = nullptr;

char* OpenclDevice::getExtensionString() {
    static std::string extStr;
    if (extStr.empty()) {
        std::stringstream ss;
        for (int i = 0; OclExtensionsString[i] != nullptr; ++i) {
            ss << OclExtensionsString[i];
        }
        extStr = ss.str();
    }

    return const_cast<char*>(extStr.c_str());
}

extern "C" CL_API_ENTRY cl_int CL_API_CALL
clGetPlatformIDs([[maybe_unused]] cl_uint           num_entries ,
                 [[maybe_unused]] cl_platform_id *  platforms ,
                 [[maybe_unused]] cl_uint *         num_platforms) CL_API_SUFFIX__VERSION_1_0
{
    cl_int return_value = CL_OUT_OF_RESOURCES;
    return return_value;
}

extern "C" CL_API_ENTRY cl_int CL_API_CALL
clGetPlatformInfo(cl_platform_id platform,
                         cl_platform_info param_name, 
                         size_t param_value_size,
                         void* param_value,
                         size_t* param_value_size_ret) {
    if (platform == nullptr || platform != damoPlatform ) {
        return CL_INVALID_PLATFORM;
    }

    int driverVersion;
    // todo: check api call
    if (dfDriverGetVersion(&driverVersion) != DF_SUCCESS) {
        return CL_INVALID_VALUE;
    }

    std::string value = "";
    switch (param_name) {
        case CL_PLATFORM_PROFILE: value = platform->profile; break;
        case CL_PLATFORM_VERSION: value = platform->version; break;
        case CL_PLATFORM_NAME: value = platform->name; break;
        case CL_PLATFORM_VENDOR: value = platform->vendor; break;
        case CL_PLATFORM_EXTENSIONS: value = platform->extensions; break;
        case CL_PLATFORM_ICD_SUFFIX_KHR: value = platform->suffix; break;
        default: return CL_INVALID_VALUE;
    }

    if (param_value_size_ret != nullptr) {
        *param_value_size_ret = value.size() + 1;
    }

    if (param_value == nullptr) {
        return CL_SUCCESS;
    }

    if (param_value_size < value.size() + 1) {
        return CL_INVALID_VALUE;
    }

    snprintf(reinterpret_cast<char*>(param_value), param_value_size, "%s", value.c_str());

    return CL_SUCCESS;
}

extern "C" CL_API_ENTRY cl_int CL_API_CALL
clGetDeviceIDs(cl_platform_id platform,
                      cl_device_type device_type,
                      cl_uint num_entries,
                      cl_device_id* devices,
                      cl_uint* num_devices) {
    if (platform == nullptr || platform != damoPlatform) {
        return CL_INVALID_PLATFORM;
    }

    if ((num_entries > 0 && devices == nullptr) ||
        (num_entries == 0 && devices != nullptr)) {
        return CL_INVALID_VALUE;
    }

    if (!((device_type & CL_DEVICE_TYPE_GPU) || (device_type == CL_DEVICE_TYPE_ALL) ||
          (device_type == CL_DEVICE_TYPE_DEFAULT))) {
        if (num_devices)
            *num_devices = 0;
        return CL_DEVICE_NOT_FOUND;
    }

    int count = 0;
    DFResult ret = dfDeviceGetCount(&count);
    if (ret != DF_SUCCESS || count == 0) {
        return CL_DEVICE_NOT_FOUND;
    }
    std::vector<std::shared_ptr<OpenclDevice>> oclDevs;
    for (int i = 0; i < count; i++) {
        DFDevice raw_device_handle = nullptr;
        DFResult ret = dfDeviceGet(&raw_device_handle, i);
        if (ret != DF_SUCCESS) {
            return CL_DEVICE_NOT_FOUND;
        }
        auto dev = std::make_shared<OpenclDevice>(raw_device_handle, i);
        oclDevs.push_back(dev);
    }

    auto it = oclDevs.cbegin();
    uint32_t minCount = std::min(num_entries, (uint32_t)oclDevs.size());
    while(minCount--) {
        *devices = as_cl(*it);
        it++;
        devices++;
        --num_entries;
    }
    while(num_entries--) {
        *devices++ = (cl_device_id)0;
    }
    if (num_devices) {
        *num_devices = (uint32_t)count;
    }
    return CL_SUCCESS;
}

extern "C" CL_API_ENTRY cl_int CL_API_CALL
clGetDeviceInfo(cl_device_id device,
                       cl_device_info param_name,
                       size_t param_value_size,
                       void* param_value,
                       size_t* param_value_size_ret) {
    cl_int result = CL_SUCCESS;
    if (!is_valid(device))
        return CL_INVALID_DEVICE;

    switch (param_name) {
        case CL_DEVICE_TYPE:
        {
            CL_SET_RETURN_VAL(
                param_value_size, param_value, param_value_size_ret, cl_device_type, CL_DEVICE_TYPE_GPU, result);
            break;
        }
        case CL_DEVICE_VENDOR_ID:
        {
            // todo: fix vendor id
            CL_SET_RETURN_VAL(
                param_value_size, param_value, param_value_size_ret, cl_uint, 0x1234, result);
            break;
        }
        case CL_DEVICE_MAX_COMPUTE_UNITS:
        {
            int64_t computeUnits = 0;
            auto oclDev = as_internal(device);
            if (dfDeviceGetAttribute(&computeUnits, DF_DEV_ATTR_PE_COUNT_IN_ONE_DIE, oclDev->get()) != DF_SUCCESS) {
                return CL_INVALID_VALUE;
            }
            CL_SET_RETURN_VAL(
                param_value_size, param_value, param_value_size_ret, cl_uint, computeUnits, result);
            break;
        }
        case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:
        {
            CL_SET_RETURN_VAL(
                param_value_size, param_value, param_value_size_ret, cl_uint, 3, result);
            break;
        }
        case CL_DEVICE_VENDOR:
        {
            const char ch[] = "DAMO Corporation";
            size_t len = strlen(ch) + 1;
            CL_SET_RETURN_PTR(
                param_value_size, param_value, param_value_size_ret, ch, len, result);
            break;
        }
        case CL_DEVICE_NAME:
        {
            const char ch[] = "DAMO H1s";
            size_t len = strlen(ch) + 1;
            CL_SET_RETURN_PTR(
                param_value_size, param_value, param_value_size_ret, ch, len, result);
            break;
        }
        case CL_DEVICE_VERSION:
        {
            const char ch[] = "OpenCL 2.0";
            CL_SET_RETURN_PTR(
                param_value_size, param_value, param_value_size_ret, ch, strlen(ch) + 1, result);
            break;
        }
        case CL_DEVICE_OPENCL_C_VERSION:
        {
            const char ch[] = "OpenCL C 2.0";
            CL_SET_RETURN_PTR(
                param_value_size, param_value, param_value_size_ret, ch, strlen(ch) + 1, result);
            break;
        }
        case CL_DEVICE_SINGLE_FP_CONFIG:
        {
            cl_device_fp_config singleFPConfig = CL_FP_ROUND_TO_NEAREST | CL_FP_ROUND_TO_ZERO | CL_FP_ROUND_TO_INF | CL_FP_INF_NAN | CL_FP_FMA | CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT | CL_FP_DENORM;
            CL_SET_RETURN_VAL(
                param_value_size, param_value, param_value_size_ret, cl_device_fp_config, singleFPConfig, result);
            break;
        }
        case CL_DEVICE_PROFILE:
        {
            const char ch[] = "EMBEDDED_PROFILE";
            CL_SET_RETURN_PTR(
                param_value_size, param_value, param_value_size_ret, ch, strlen(ch) + 1, result);
            break;
        }
        case CL_DEVICE_EXTENSIONS:
        {
            char* externsions = OpenclDevice::getExtensionString();
            CL_SET_RETURN_PTR(
                param_value_size, param_value, param_value_size_ret, externsions, strlen(externsions) + 1, result);
            break;
        }
        case CL_DEVICE_ADDRESS_BITS:
        {
            CL_SET_RETURN_VAL(param_value_size, param_value, param_value_size_ret, cl_uint, 64, result);
            break;
        }
        case CL_DEVICE_PLATFORM:
        {
            CL_SET_RETURN_PTR(param_value_size, param_value, param_value_size_ret, &damoPlatform, sizeof(cl_platform_id), result);
            break;
        }
        default: result = CL_INVALID_VALUE;
    }
    return result;
}

extern CLIicdDispatchTable *g_icd_dispatchTable;
extern "C" cl_int cliIcdDispatchTableCreate();

static cl_int initialize_once() {
    cl_int result = CL_SUCCESS;
    if (g_initialized) return result;

    DFResult ret = dfInit(0);
    if (ret != DF_SUCCESS) {
        return CL_INVALID_PLATFORM;
    }

    result = cliIcdDispatchTableCreate();
    damoPlatform = (cl_platform_id)malloc(sizeof(struct _cl_platform_id));
    memset(damoPlatform, 0, sizeof(struct _cl_platform_id));

    CL_INIT_PLATFORM(damoPlatform, g_icd_dispatchTable);
    damoPlatform->version = "OpenCL 1.2 DAMO 1.0";
    damoPlatform->vendor = "DAMO Corporation";
    damoPlatform->profile = "EMBEDDED_PROFILE";
    damoPlatform->name = "DAMO Accelerated Compute Engine";
    damoPlatform->extensions = "cl_khr_il_program cl_khr_fp64";
    damoPlatform->suffix = "DAMO";
    g_initialized = CL_TRUE;
    
    return result;
}

extern "C" CL_API_ENTRY cl_int CL_API_CALL
clIcdGetPlatformIDsKHR(cl_uint num_entries,
                       cl_platform_id* platforms,
                       cl_uint* num_platforms) {
    cl_int init_ret = initialize_once();
    if (init_ret != CL_SUCCESS) {
        return init_ret;
    }

    if (num_platforms) {
        *num_platforms = 1;
    }

    if (platforms != nullptr && num_entries > 0) {
        platforms[0] = damoPlatform;
    }

    if (num_entries > 0 && platforms == nullptr) {
        return CL_INVALID_VALUE;
    }

    return CL_SUCCESS;
}