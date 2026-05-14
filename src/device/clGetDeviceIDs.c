#include <string.h>
#include <stdio.h>
#include "cl_helper.h"
#include "cl_util.h"
#include "dfruntime/dfruntime.h"

extern cl_platform_id thrivePlatform;

CL_API_ENTRY cl_int CL_API_CALL clGetDeviceIDs(cl_platform_id platform, cl_device_type device_type,
                                               cl_uint num_entries, cl_device_id *devices,
                                               cl_uint *num_devices) CL_API_SUFFIX__VERSION_1_0 {
  printf("=== [DEBUG] clGetDeviceIDs CALLED ===\n");
  /* Validate platform */
  if (platform == NULL || platform != thrivePlatform) {
    return CL_INVALID_PLATFORM;
  }

  /* Validate device_type */
  if (device_type == 0) {
    return CL_INVALID_VALUE;
  }

  /* Initialize devices if not already done */
  cl_int ret = dfcl_init_devices(platform);
  if (ret != CL_SUCCESS) {
    return ret;
  }

  /* Query available devices matching the type */
  cl_uint total_devices = dfcl_get_device_type_count(device_type);
  if (total_devices == 0) {
    return CL_DEVICE_NOT_FOUND;
  }

  /* Return device count if requested */
  if (num_devices != NULL) {
    *num_devices = total_devices;
  }

  /* Return device IDs if buffer provided */
  if (devices != NULL) {
    if (num_entries == 0) {
      return CL_INVALID_VALUE;
    }
    cl_uint added = dfcl_get_devices(device_type, devices, num_entries);
    if (added == 0) {
      return CL_DEVICE_NOT_FOUND;
    }
  }

  return CL_SUCCESS;
}
