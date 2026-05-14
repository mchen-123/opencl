#include <stdatomic.h>
#include "cl_helper.h"
#include "cl_util.h"

CL_API_ENTRY cl_int CL_API_CALL clRetainDevice(cl_device_id device) CL_API_SUFFIX__VERSION_1_2 {
  if (device == NULL) return CL_INVALID_DEVICE;

  if (device->available != CL_TRUE) return CL_DEVICE_NOT_AVAILABLE;

  /* Increment reference count for both root and sub-devices */
  dfcl_retain_object(device);

  DFCL_MSG_INFO("Retain Device %d (%p), Refcount now: %d", device->dev_id, (void *)device,
                atomic_load(&((ThiveclObject *)device)->refcount));
  return CL_SUCCESS;
}
