#include "cl_helper.h"
#include "cl_util.h"

CL_API_ENTRY cl_int CL_API_CALL clReleaseDevice(cl_device_id device) CL_API_SUFFIX__VERSION_1_2 {
  if (device == NULL) return CL_INVALID_DEVICE;

  /* Root devices are not released (managed by platform) */
  if (device->parent_device == NULL) {
    return CL_SUCCESS;
  }

  /* Decrement reference count */
  int new_refcount = dfcl_release_object(device);

  DFCL_MSG_INFO("Release Device %d (%p), Refcount now: %d", device->dev_id, (void *)device,
                new_refcount);

  /* Free device memory when refcount reaches 0 */
  if (new_refcount <= 0) {
    /* Release parent device reference */
    if (device->parent_device) {
      clReleaseDevice(device->parent_device);
    }
    DFCL_MEM_FREE(device);
  }

  return CL_SUCCESS;
}
