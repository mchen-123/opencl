#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "cl_helper.h"
#include "cl_util.h"

cl_platform_id thrivePlatform = NULL;
static cl_bool g_initialized = CL_FALSE;

extern CLIicdDispatchTable *g_icd_dispatchTable;
extern cl_int cliIcdDispatchTableCreate(void);

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
