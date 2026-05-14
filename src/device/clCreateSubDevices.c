#include <string.h>
#include "cl_helper.h"
#include "cl_util.h"

CL_API_ENTRY cl_int CL_API_CALL clCreateSubDevices(
    cl_device_id in_device, const cl_device_partition_property *properties, cl_uint num_devices,
    cl_device_id *out_devices, cl_uint *num_devices_ret) CL_API_SUFFIX__VERSION_1_2 {
  cl_int errcode = CL_SUCCESS;
  /* Validate input parameters */
  if (in_device == NULL || properties == NULL) return CL_INVALID_VALUE;

  /* Only root devices can be partitioned */
  if (in_device->parent_device != NULL) return CL_INVALID_DEVICE;

  /* Check if device supports partitioning */
  if (!in_device->is_multi_die || in_device->num_dies == 0) return CL_DEVICE_PARTITION_FAILED;

  cl_device_partition_property ptype = properties[0];
  cl_uint sub_count = 0;
  cl_uint prop_idx = 1;

  /* Calculate number of sub-devices based on partition type */
  switch (ptype) {
  case CL_DEVICE_PARTITION_BY_NAMES_EXT: {
    /* Count dies specified in the property list */
    for (cl_uint i = 1; properties[i] != 0; i++) {
      if (properties[i] >= (cl_device_partition_property)in_device->num_dies)
        return CL_INVALID_VALUE;
      sub_count++;
    }
    break;
  }

  default:
    return CL_INVALID_VALUE;
  }

  /* Return sub-device count if requested */
  if (num_devices_ret) *num_devices_ret = sub_count;
  /* Early return if no output buffer or num_devices is 0 */
  if (num_devices == 0 || out_devices == NULL) return CL_SUCCESS;
  /* Create sub-devices */
  cl_uint created = 0;
  for (cl_uint i = 0; i < sub_count && i < num_devices; i++) {
    _cl_device_id *sub = DFCL_NEW(_cl_device_id);
    DFCL_GOTO_ERROR_COND(!sub, CL_OUT_OF_HOST_MEMORY);

    dfcl_init_object(sub, in_device->dev_id * 100 + i);
    /* Copy parent device properties (skip base object and dev_id) */
    memcpy(&sub->type, &in_device->type,
           sizeof(struct _cl_device_id) - offsetof(struct _cl_device_id, type));

    dfcl_init_object(sub, in_device->dev_id * 100 + i);
    /* Initialize sub-device specific fields */
    sub->parent_device = in_device;
    sub->dev_id = in_device->dev_id * 100 + i;
    sub->is_multi_die = CL_FALSE;
    sub->num_dies = 1;
    sub->available = CL_TRUE;

    uint32_t target_die = (ptype == CL_DEVICE_PARTITION_BY_NAMES_EXT) ? properties[prop_idx++] : i;
    /* Calculate die grid coordinates */
    int x = target_die % 2;
    int y = target_die / 2;

    sub->die_grid.upperLeft = (DFDieCoord){x, y};
    sub->die_grid.bottomRight = (DFDieCoord){x, y};

    out_devices[i] = sub;
    created++;

    /* Retain parent device for each sub-device created */
    clRetainDevice(in_device);
  }

  return CL_SUCCESS;

ERROR:
  /* Clean up already-created sub-devices on failure */
  for (cl_uint j = 0; j < created; j++) {
    if (out_devices[j]) {
      DFCL_MEM_FREE(out_devices[j]);
    }
  }
  return errcode;
}
