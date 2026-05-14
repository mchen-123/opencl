#include <string.h>
#include <stdio.h>

#include "cl_helper.h"
#include "cl_util.h"

extern cl_platform_id thrivePlatform;

/* cl_platform_info */
#define CL_PLATFORM_ICD_SUFFIX_KHR 0x0920

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
