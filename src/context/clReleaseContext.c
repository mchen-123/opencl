#include <stdatomic.h>
#include "cl_util.h"
#include "cl_helper.h"

extern atomic_int context_c;

CL_API_ENTRY cl_int CL_API_CALL clReleaseContext(cl_context context) CL_API_SUFFIX__VERSION_1_0 {
  if (!IS_VALID_OBJECT(context)) {
    return CL_INVALID_CONTEXT;
  }

  int new_refcount = dfcl_release_object(context);
  DFCL_MSG_INFO("Release Context %p, Refcount now: %d", (void *)context, new_refcount);

  if (new_refcount == 0) {
    DFCL_ATOMIC_DEC(&context_c);

    /* Release all command queues */
    while (context->command_queues) {
      cl_command_queue cq = context->command_queues;
      DL_DELETE(context->command_queues, cq);
      clReleaseCommandQueue(cq);
    }

    /* Release default queues */
    if (context->default_queues) {
      for (uint32_t i = 0; i < context->num_devices; i++) {
        if (context->default_queues[i]) {
          clReleaseCommandQueue(context->default_queues[i]);
        }
      }
      DFCL_MEM_FREE(context->default_queues);
    }

    /* Release device references */
    for (uint32_t i = 0; i < context->num_create_devices; i++) {
      clReleaseDevice(context->create_devices[i]);
    }

    DFCL_MEM_FREE(context->devices);
    DFCL_MEM_FREE(context->create_devices);
    DFCL_MEM_FREE(context->properties);

    DFCL_DESTROY_OBJECT(context);
    DFCL_MEM_FREE(context);
  }

  return CL_SUCCESS;
}
