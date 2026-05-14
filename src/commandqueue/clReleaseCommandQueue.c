#include <stdatomic.h>

#include "cl_util.h"
#include "cl_helper.h"
#include "dfruntime/dfruntime.h"

extern atomic_int queue_c;

CL_API_ENTRY cl_int CL_API_CALL clReleaseCommandQueue(cl_command_queue command_queue) {
  if (!IS_VALID_OBJECT(command_queue)) {
    return CL_INVALID_COMMAND_QUEUE;
  }

  int new_refcount = dfcl_release_object(command_queue);
  DFCL_MSG_INFO("Release Command Queue= %p, new_refcount =%d", command_queue, new_refcount);

  if (new_refcount == 0) {
    cl_context context = command_queue->context;
    cl_device_id device = command_queue->device;

    DFCL_ATOMIC_DEC(&queue_c);

    /* Remove from context's command queue list and release context reference */
    if ((command_queue->properties & CL_QUEUE_HIDDEN) == 0) {
      DFCL_LOCK_OBJ(context);
      context_remove_command_queue(context, command_queue);
      DFCL_UNLOCK_OBJ(context);
      clReleaseContext(context);
    }

    /* Free DF runtime resources (stream, queue data) */
    if (device->available == CL_TRUE) {
      dfcl_dfruntime_free_queue(device, command_queue);
    }

    DFCL_DESTROY_OBJECT(command_queue);
    DFCL_MEM_FREE(command_queue);
  }

  return CL_SUCCESS;
}
