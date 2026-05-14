#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "cl_helper.h"
#include "cl_util.h"

cl_platform_id thrivePlatform = NULL;
static cl_bool g_initialized = CL_FALSE;

extern CLIicdDispatchTable *g_icd_dispatchTable;
extern cl_int cliIcdDispatchTableCreate(void);

/* cl_platform_info */
#define CL_PLATFORM_ICD_SUFFIX_KHR 0x0920

CL_API_ENTRY cl_int CL_API_CALL clGetPlatformIDs(cl_uint num_entries, cl_platform_id *platforms,
                                                 cl_uint *num_platforms)
    CL_API_SUFFIX__VERSION_1_0 {
  return clIcdGetPlatformIDsKHR(num_entries, platforms, num_platforms);
}

CL_API_ENTRY cl_int CL_API_CALL
clGetPlatformInfo(cl_platform_id platform, cl_platform_info param_name, size_t param_value_size,
                  void *param_value, size_t *param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  const char *value = NULL;
  size_t value_len = 0;

  if (platform == NULL || platform != thrivePlatform) {
    return CL_INVALID_PLATFORM;
  }

  switch (param_name) {
  case CL_PLATFORM_PROFILE:
    value = platform->profile;
    break;
  case CL_PLATFORM_VERSION:
    value = platform->version;
    break;
  case CL_PLATFORM_NAME:
    value = platform->name;
    break;
  case CL_PLATFORM_VENDOR:
    value = platform->vendor;
    break;
  case CL_PLATFORM_EXTENSIONS:
    value = platform->extensions;
    break;
  case CL_PLATFORM_ICD_SUFFIX_KHR:
    value = platform->suffix;
    break;
  default:
    return CL_INVALID_VALUE;
  }

  if (value == NULL) {
    return CL_INVALID_VALUE;
  }

  value_len = strlen(value) + 1;

  if (param_value_size_ret != NULL) {
    *param_value_size_ret = value_len;
  }

  if (param_value == NULL) {
    return CL_SUCCESS;
  }

  if (param_value_size < value_len) {
    return CL_INVALID_VALUE;
  }

  snprintf((char *)param_value, param_value_size, "%s", value);

  return CL_SUCCESS;
}

static cl_int initialize_once(void) {
  cl_int result = CL_SUCCESS;
  if (g_initialized) return result;

  result = cliIcdDispatchTableCreate();
  if (result != CL_SUCCESS) return result;

  thrivePlatform = (cl_platform_id)DFCL_NEW(struct _cl_platform_id);
  if (thrivePlatform == NULL) return CL_OUT_OF_HOST_MEMORY;

  dfcl_init_object(thrivePlatform, 1);
  thrivePlatform->version = "OpenCL 3.0 THRIVE 1.0";
  thrivePlatform->vendor = "THRIVE Corporation";
  thrivePlatform->profile = "EMBEDDED_PROFILE";
  thrivePlatform->name = "THRIVE Accelerated Compute Engine";
  thrivePlatform->extensions = "cl_khr_il_program cl_khr_fp64";
  thrivePlatform->suffix = "THRIVE";
  g_initialized = CL_TRUE;

  return result;
}

CL_API_ENTRY cl_int CL_API_CALL clIcdGetPlatformIDsKHR(cl_uint num_entries,
                                                       cl_platform_id *platforms,
                                                       cl_uint *num_platforms) {
  cl_int init_ret = initialize_once();
  if (init_ret != CL_SUCCESS) {
    return init_ret;
  }

  if (num_platforms) {
    *num_platforms = 1;
  }

  if (platforms != NULL && num_entries > 0) {
    platforms[0] = thrivePlatform;
  }

  if (num_entries > 0 && platforms == NULL) {
    return CL_INVALID_VALUE;
  }

  return CL_SUCCESS;
}
