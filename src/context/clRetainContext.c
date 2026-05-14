#include <stdatomic.h>
#include "cl_util.h"
#include "cl_helper.h"

CL_API_ENTRY cl_int CL_API_CALL clRetainContext(cl_context context) CL_API_SUFFIX__VERSION_1_0 {
  if (!IS_VALID_OBJECT(context)) {
    return CL_INVALID_CONTEXT;
  }

  int ret = dfcl_retain_object(context);
  if (ret != CL_SUCCESS) {
    return CL_INVALID_CONTEXT;
  }

  DFCL_MSG_INFO("Retain Context %p, Refcount now: %d", (void *)context,
                atomic_load(&((ThiveclObject *)context)->refcount));
  return CL_SUCCESS;
}
