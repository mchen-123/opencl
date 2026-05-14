#include <CL/cl.h>
#include <CL/cl_ext.h>
#include <cl_khr_icd2.h>
#include <string.h>

/* Forward declarations */
extern cl_int clIcdGetPlatformIDsKHR(cl_uint num_entries, cl_platform_id *platforms,
                                     cl_uint *num_platforms);

CL_API_ENTRY void *CL_API_CALL clGetExtensionFunctionAddress(const char *name) {
  if (name == NULL) return NULL;

  if (strcmp(name, "clIcdGetPlatformIDsKHR") == 0) {
    return (void *)clIcdGetPlatformIDsKHR;
  }

  return NULL;
}
