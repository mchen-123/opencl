#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "cl_helper.h"
#include "cl_util.h"
#include "devices.h"

extern cl_platform_id thrivePlatform;

/* ==================== clGetDeviceIDs ==================== */
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

/* ==================== clGetDeviceInfo ==================== */
CL_API_ENTRY cl_int CL_API_CALL
clGetDeviceInfo(cl_device_id device, cl_device_info param_name, size_t param_value_size,
                void *param_value, size_t *param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  if (!IS_VALID_OBJECT(device)) return CL_INVALID_DEVICE;

  switch (param_name) {
  /* === Basic Device Info === */
  case CL_DEVICE_TYPE:
    DFCL_RETURN_VALUE(device->type, cl_device_type);
    break;

  case CL_DEVICE_VENDOR_ID:
    DFCL_RETURN_VALUE(device->vendor_id, cl_uint);
    break;

  case CL_DEVICE_MAX_COMPUTE_UNITS:
    DFCL_RETURN_VALUE(device->max_compute_units, cl_uint);
    break;

  case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:
    DFCL_RETURN_VALUE(device->max_work_item_dimensions, cl_uint);
    break;

  case CL_DEVICE_MAX_WORK_GROUP_SIZE:
    /* TODO: Return actual max work group size */
    DFCL_RETURN_VALUE(1024, size_t);
    break;

  case CL_DEVICE_MAX_WORK_ITEM_SIZES:
    /* TODO: Return actual max work item sizes */
    {
      size_t sizes[3] = {1024, 1024, 1024};
      size_t len = sizeof(sizes);
      if (param_value_size_ret) *param_value_size_ret = len;
      if (param_value) {
        if (param_value_size < len) return CL_INVALID_VALUE;
        memcpy(param_value, sizes, len);
      }
    }
    break;

  /* === Memory & Addressing === */
  case CL_DEVICE_ADDRESS_BITS:
    DFCL_RETURN_VALUE(device->address_bits, cl_uint);
    break;

  case CL_DEVICE_MAX_MEM_ALLOC_SIZE:
    /* TODO: Return actual max alloc size */
    DFCL_RETURN_VALUE(128 * 1024 * 1024, cl_ulong);
    break;

  case CL_DEVICE_GLOBAL_MEM_SIZE:
    /* TODO: Return actual global memory size */
    DFCL_RETURN_VALUE(0, cl_ulong);
    break;

  case CL_DEVICE_LOCAL_MEM_SIZE:
    /* TODO: Return actual local memory size */
    DFCL_RETURN_VALUE(32768, cl_ulong);
    break;

  case CL_DEVICE_LOCAL_MEM_TYPE:
    DFCL_RETURN_VALUE(CL_LOCAL, cl_device_local_mem_type);
    break;

  /* === Floating Point Config === */
  case CL_DEVICE_SINGLE_FP_CONFIG:
    DFCL_RETURN_VALUE(device->single_fp_config, cl_device_fp_config);
    break;

  case CL_DEVICE_DOUBLE_FP_CONFIG:
    /* TODO: Return double FP config if supported */
    DFCL_RETURN_VALUE(0, cl_device_fp_config);
    break;

  case CL_DEVICE_HALF_FP_CONFIG:
    /* TODO: Return half FP config if supported */
    DFCL_RETURN_VALUE(0, cl_device_fp_config);
    break;

  /* === Device Identity Strings === */
  case CL_DEVICE_VENDOR:
    DFCL_RETURN_STRING(device->vendor ? device->vendor : "Unknown");
    break;

  case CL_DEVICE_NAME:
    DFCL_RETURN_STRING(device->long_name ? device->long_name : "Unknown");
    break;

  case CL_DEVICE_VERSION:
    DFCL_RETURN_STRING(device->version ? device->version : "OpenCL 3.0");
    break;

  case CL_DRIVER_VERSION:
    DFCL_RETURN_STRING(device->driver_version ? device->driver_version : "1.0");
    break;

  case CL_DEVICE_PROFILE:
    DFCL_RETURN_STRING(device->profile ? device->profile : "FULL_PROFILE");
    break;

  case CL_DEVICE_OPENCL_C_VERSION:
    DFCL_RETURN_STRING("OpenCL C 3.0");
    break;

  /* === Platform === */
  case CL_DEVICE_PLATFORM:
    DFCL_RETURN_VALUE(device->platform ? device->platform : thrivePlatform, cl_platform_id);
    break;

  /* === Extensions === */
  case CL_DEVICE_EXTENSIONS:
    DFCL_RETURN_STRING("");
    break;

  /* === Queue Properties === */
  case CL_DEVICE_QUEUE_ON_HOST_PROPERTIES:
    /* TODO: Return queue properties */
    {
      cl_command_queue_properties props =
          CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE;
      DFCL_RETURN_VALUE(props, cl_command_queue_properties);
    }
    break;

  /* === Preferred Vector Widths === */
  case CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:
    DFCL_RETURN_VALUE(16, cl_uint);
    break;

  case CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:
    DFCL_RETURN_VALUE(8, cl_uint);
    break;

  case CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT:
    DFCL_RETURN_VALUE(4, cl_uint);
    break;

  case CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG:
    DFCL_RETURN_VALUE(2, cl_uint);
    break;

  case CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:
    DFCL_RETURN_VALUE(4, cl_uint);
    break;

  case CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:
    DFCL_RETURN_VALUE(2, cl_uint);
    break;

  case CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF:
    DFCL_RETURN_VALUE(8, cl_uint);
    break;

  /* === Native Vector Widths === */
  case CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR:
    DFCL_RETURN_VALUE(16, cl_uint);
    break;

  case CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT:
    DFCL_RETURN_VALUE(8, cl_uint);
    break;

  case CL_DEVICE_NATIVE_VECTOR_WIDTH_INT:
    DFCL_RETURN_VALUE(4, cl_uint);
    break;

  case CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG:
    DFCL_RETURN_VALUE(2, cl_uint);
    break;

  case CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT:
    DFCL_RETURN_VALUE(4, cl_uint);
    break;

  case CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE:
    DFCL_RETURN_VALUE(2, cl_uint);
    break;

  case CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF:
    DFCL_RETURN_VALUE(8, cl_uint);
    break;

  /* === Device Partition Properties (for sub-devices) === */
  case CL_DEVICE_PARENT_DEVICE:
    DFCL_RETURN_VALUE(device->parent_device, cl_device_id);
    break;

  case CL_DEVICE_PARTITION_MAX_SUB_DEVICES:
    DFCL_RETURN_VALUE(device->num_dies > 0 ? device->num_dies : 1, cl_uint);
    break;

  /* === Sub-device specific info === */
  case CL_DEVICE_REFERENCE_COUNT:
    DFCL_RETURN_VALUE((cl_uint)atomic_load(&device->base.refcount), cl_uint);
    break;

  case CL_DEVICE_AVAILABLE:
    DFCL_RETURN_VALUE(device->available, cl_bool);
    break;

  case CL_DEVICE_COMPILER_AVAILABLE:
    DFCL_RETURN_VALUE(CL_FALSE, cl_bool);
    break;

  case CL_DEVICE_LINKER_AVAILABLE:
    DFCL_RETURN_VALUE(CL_FALSE, cl_bool);
    break;

  case CL_DEVICE_ENDIAN_LITTLE:
    DFCL_RETURN_VALUE(CL_TRUE, cl_bool);
    break;

  case CL_DEVICE_EXECUTION_CAPABILITIES:
    DFCL_RETURN_VALUE(CL_EXEC_KERNEL, cl_device_exec_capabilities);
    break;

  case CL_DEVICE_PROFILING_TIMER_RESOLUTION:
    DFCL_RETURN_VALUE(1, size_t);
    break;

  case CL_DEVICE_GLOBAL_MEM_CACHE_SIZE:
    DFCL_RETURN_VALUE(0, cl_ulong);
    break;

  case CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE:
    DFCL_RETURN_VALUE(0, cl_uint);
    break;

  case CL_DEVICE_MAX_CLOCK_FREQUENCY:
    DFCL_RETURN_VALUE(1000, cl_uint);
    break;

  case CL_DEVICE_MAX_READ_IMAGE_ARGS:
    DFCL_RETURN_VALUE(128, cl_uint);
    break;

  case CL_DEVICE_MAX_WRITE_IMAGE_ARGS:
    DFCL_RETURN_VALUE(8, cl_uint);
    break;

  case CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS:
    DFCL_RETURN_VALUE(8, cl_uint);
    break;

  case CL_DEVICE_MAX_SAMPLERS:
    DFCL_RETURN_VALUE(16, cl_uint);
    break;

  case CL_DEVICE_MEM_BASE_ADDR_ALIGN:
    DFCL_RETURN_VALUE(128, cl_uint);
    break;

  case CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE:
    DFCL_RETURN_VALUE(128, cl_uint);
    break;

  case CL_DEVICE_MAX_PARAMETER_SIZE:
    DFCL_RETURN_VALUE(1024, size_t);
    break;

  case CL_DEVICE_PRINTF_BUFFER_SIZE:
    DFCL_RETURN_VALUE(1024 * 1024, size_t);
    break;

  case CL_DEVICE_PREFERRED_INTEROP_USER_SYNC:
    DFCL_RETURN_VALUE(CL_TRUE, cl_bool);
    break;

  case CL_DEVICE_HOST_UNIFIED_MEMORY:
    DFCL_RETURN_VALUE(CL_FALSE, cl_bool);
    break;

    /* CL_DEVICE_CORRECTLY_ROUNDED_DIVIDE_SQRT not available in OpenCL 3.0 headers */

  default:
    return CL_INVALID_VALUE;
  }

  return CL_SUCCESS;
}

/* ==================== clRetainDevice ==================== */
CL_API_ENTRY cl_int CL_API_CALL clRetainDevice(cl_device_id device) CL_API_SUFFIX__VERSION_1_2 {
  if (device == NULL) return CL_INVALID_DEVICE;

  if (device->available != CL_TRUE) return CL_DEVICE_NOT_AVAILABLE;

  /* Increment reference count for both root and sub-devices */
  dfcl_retain_object(device);

  DFCL_MSG_INFO("Retain Device %d (%p), Refcount now: %d", device->dev_id, (void *)device,
                atomic_load(&((ThiveclObject *)device)->refcount));
  return CL_SUCCESS;
}

/* ==================== clReleaseDevice ==================== */
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

/* ==================== clCreateSubDevices ==================== */
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
