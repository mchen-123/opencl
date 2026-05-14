#include <string.h>
#include <stdio.h>
#include "cl_helper.h"
#include "cl_util.h"

extern cl_platform_id thrivePlatform;

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
    DFCL_RETURN_VALUE(1024, size_t);
    break;

  case CL_DEVICE_MAX_WORK_ITEM_SIZES: {
    size_t sizes[3] = {1024, 1024, 1024};
    size_t len = sizeof(sizes);
    if (param_value_size_ret) *param_value_size_ret = len;
    if (param_value) {
      if (param_value_size < len) return CL_INVALID_VALUE;
      memcpy(param_value, sizes, len);
    }
  } break;

  /* === Memory & Addressing === */
  case CL_DEVICE_ADDRESS_BITS:
    DFCL_RETURN_VALUE(device->address_bits, cl_uint);
    break;

  case CL_DEVICE_MAX_MEM_ALLOC_SIZE:
    DFCL_RETURN_VALUE(128 * 1024 * 1024, cl_ulong);
    break;

  case CL_DEVICE_GLOBAL_MEM_SIZE:
    DFCL_RETURN_VALUE(0, cl_ulong);
    break;

  case CL_DEVICE_LOCAL_MEM_SIZE:
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
    DFCL_RETURN_VALUE(0, cl_device_fp_config);
    break;

  case CL_DEVICE_HALF_FP_CONFIG:
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
  case CL_DEVICE_QUEUE_ON_HOST_PROPERTIES: {
    cl_command_queue_properties props =
        CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE;
    DFCL_RETURN_VALUE(props, cl_command_queue_properties);
  } break;

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

  /* === Device Partition Properties === */
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

  default:
    return CL_INVALID_VALUE;
  }

  return CL_SUCCESS;
}
