#include "cl_helper.h"
#include "cl_util.h"

extern cl_int clIcdGetPlatformIDsKHR(cl_uint num_entries, cl_platform_id *platforms,
                                     cl_uint *num_platforms);

CL_API_ENTRY cl_int CL_API_CALL clGetPlatformIDs(cl_uint num_entries, cl_platform_id *platforms,
                                                 cl_uint *num_platforms)
    CL_API_SUFFIX__VERSION_1_0 {
  return clIcdGetPlatformIDsKHR(num_entries, platforms, num_platforms);
}
