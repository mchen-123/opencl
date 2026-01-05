#include <string>
#include <cstdio>
#include <memory>
#include <cstring>

#include "dfcl_cl.hpp"
#include "cl_rename_api.hpp"

cl_platform_id damoPlatform = nullptr;
static cl_bool g_initialized = CL_FALSE;

extern CLIicdDispatchTable *g_icd_dispatchTable;
extern "C" cl_int cliIcdDispatchTableCreate();

/* cl_platform_info */
#define CL_PLATFORM_ICD_SUFFIX_KHR                          0x0920

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
                         size_t* param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
    if (platform == nullptr || platform != damoPlatform ) {
        return CL_INVALID_PLATFORM;
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

static cl_int initialize_once() {
    cl_int result = CL_SUCCESS;
    if (g_initialized) return result;

    result = cliIcdDispatchTableCreate();
    damoPlatform = (cl_platform_id)malloc(sizeof(struct _cl_platform_id));
    memset(damoPlatform, 0, sizeof(struct _cl_platform_id));

    CL_INIT_PLATFORM(damoPlatform, g_icd_dispatchTable);
    damoPlatform->version = "OpenCL 3.0 DAMO 1.0";
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