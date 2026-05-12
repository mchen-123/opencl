#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>
#include <stdbool.h>

#include "devices.h"
#include "cl_helper.h"
#include "cl_util.h"

/* ====================== 全局状态 ====================== */
dfcl_lock_t g_devices_lock = PTHREAD_MUTEX_INITIALIZER;
static cl_bool g_devices_initialized = CL_FALSE;

static int g_num_devices = 0;
static _cl_device_id **g_all_devices = NULL;

static _Atomic(_cl_device_id *) dfcl_devices_list = NULL;

static inline bool dfca_check_error(DFResult result, const char *api, const char *file, int line) {
  if (result == DF_SUCCESS) return false;

  const char *err_name = NULL;
  const char *err_msg = NULL;
  dfGetErrorName(result, &err_name);
  dfGetErrorString(result, &err_msg);

  fprintf(stderr, "[%s:%d] Runtime error during %s: %s(%s)\n", file, line, api,
          err_name ? err_name : "NULL", err_msg ? err_msg : "NULL");

  return true;
}

int dfcl_dfruntime_probe(void) {
  int probe_count = 0;
  DFResult ret = dfInit(0);
  if (ret == DF_SUCCESS) {
    ret = dfDeviceGetCount(&probe_count);
    if (ret != DF_SUCCESS) {
      probe_count = 0;
    }
  }
  return probe_count;
}

cl_int dfcl_dfruntime_init(unsigned int dev_id, _cl_device_id *dev) {
  DFResult result;
  cl_int ret = CL_SUCCESS;

  assert(dev != NULL);
  assert(dev->data == NULL);

  /* Allocate private driver data */
  dfcl_dfruntime_device_data_t *data =
      (dfcl_dfruntime_device_data_t *)calloc(1, sizeof(dfcl_dfruntime_device_data_t));
  if (data == NULL) {
    return CL_OUT_OF_HOST_MEMORY;
  }

  result = dfDeviceGet(&data->device, dev_id);
  if (DFCA_CHECK_ERROR(result, "dfDeviceGet")) {
    free(data);
    return CL_INVALID_DEVICE;
  }

  /* Basic OpenCL Properties */
  dev->vendor = "THRIVE Corporation";
  dev->vendor_id = 0x1234;
  dev->type = CL_DEVICE_TYPE_GPU;
  dev->address_bits = (sizeof(void *) * 8);
  dev->max_work_item_dimensions = 3;
  dev->image_support = CL_FALSE;
  dev->profile = "FULL_PROFILE";
  dev->parent_device = NULL;

  dev->long_name = dev->short_name = "THRIVE H1s";
  dev->version = "OpenCL 3.0 THRIVE 1.0";
  dev->driver_version = "0.6.0";

  /* Single precision floating point capabilities */
  dev->single_fp_config = CL_FP_ROUND_TO_NEAREST | CL_FP_ROUND_TO_ZERO | CL_FP_ROUND_TO_INF |
      CL_FP_INF_NAN | CL_FP_FMA | CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT | CL_FP_DENORM;

  /* ====================== Multi-Die Information ====================== */
  DFDieGrid grid = {{0, 0}, {0, 0}};
  result = dfDeviceGetDieGrid(data->device, &grid);
  if (result == DF_SUCCESS && (grid.bottomRight.x > 0 || grid.bottomRight.y > 0)) {
    dev->is_multi_die = true;
    dev->max_compute_units = dev->num_dies = (grid.bottomRight.x + 1) * (grid.bottomRight.y + 1);
    dev->die_grid = grid;

    DFCL_MSG_INFO("Device %d: Multi-Die enabled, grid=(%d,%d)-(%d,%d), total Dies=%u", dev_id,
                  grid.upperLeft.x, grid.upperLeft.y, grid.bottomRight.x, grid.bottomRight.y,
                  dev->num_dies);
  } else {
    dev->is_multi_die = false;
    dev->num_dies = 1;
    DFCL_MSG_INFO("Device %d: Single Die mode", dev_id);
  }

  /* ====================== Create Context ====================== */
  result = dfCtxCreate(&data->context, 0, data->device);
  if (DFCA_CHECK_ERROR(result, "dfCtxCreate")) {
    free(data);
    return CL_INVALID_DEVICE;
  }

  /* Attach private data */
  dev->data = data;
  data->available = CL_TRUE;
  dev->available = CL_TRUE;

  DFCL_MSG_INFO("Device %d initialized successfully (Multi-Die: %s)", dev_id,
                dev->is_multi_die ? "Yes" : "No");
  return CL_SUCCESS;
}

/* ====================== 链表辅助函数 ====================== */
static void ll_append_atomic(_cl_device_id *new_node) {
  assert(new_node != NULL);
  new_node->next = NULL;

  _cl_device_id *last = atomic_load(&dfcl_devices_list);
  _cl_device_id *next = NULL;

  if (last == NULL) {
    if (atomic_compare_exchange_weak_explicit(&dfcl_devices_list, &last, new_node,
                                              memory_order_release, memory_order_relaxed)) {
      return;
    }
  }

  do {
    while ((next = atomic_load_explicit(&last->next, memory_order_acquire)) != NULL) {
      last = next;
    }
  } while (!atomic_compare_exchange_weak_explicit(&last->next, &next, new_node,
                                                  memory_order_release, memory_order_relaxed));
}

/* ====================== Device Initialization ====================== */
cl_int dfcl_init_devices(cl_platform_id platform) {
  if (g_devices_initialized) return CL_SUCCESS;

  DFCL_LOCK(g_devices_lock);
  if (g_devices_initialized) {
    DFCL_UNLOCK(g_devices_lock);
    return CL_SUCCESS;
  }

  /* Probe Device Count */
  g_num_devices = dfcl_dfruntime_probe();
  if (g_num_devices <= 0) {
    DFCL_UNLOCK(g_devices_lock);
    return CL_DEVICE_NOT_FOUND;
  }

  /* Allocate device pointer array */
  g_all_devices = calloc(g_num_devices, sizeof(_cl_device_id *));
  if (!g_all_devices) {
    g_num_devices = 0;
    DFCL_UNLOCK(g_devices_lock);
    return CL_OUT_OF_HOST_MEMORY;
  }

  /* Create and initialize each device */
  for (int i = 0; i < g_num_devices; i++) {
    _cl_device_id *dev = calloc(1, sizeof(_cl_device_id));
    if (!dev) {
      goto error;
    }

    dev->dev_id = i;
    cl_int ret = dfcl_dfruntime_init(i, dev);
    if (ret != CL_SUCCESS) {
      free(dev);
      goto error;
    }

    CL_INIT_OBJECT(dev, platform);
    atomic_init(&dev->refcount, 1);

    g_all_devices[i] = dev;
    ll_append_atomic(dev);
  }

  g_devices_initialized = CL_TRUE;
  DFCL_MSG_INFO("Successfully initialized %d self-developed GPU(s)", g_num_devices);
  DFCL_UNLOCK(g_devices_lock);
  return CL_SUCCESS;

error:
  DFCL_UNLOCK(g_devices_lock);
  return CL_DEVICE_NOT_FOUND;
}

/* ====================== Device Query ====================== */
uint32_t dfcl_get_device_type_count(cl_device_type device_type) {
  uint32_t count = 0;
  _cl_device_id *dev = atomic_load(&dfcl_devices_list);

  while (dev != NULL) {
    if (dev->available == CL_FALSE) {
      dev = dev->next;
      continue;
    }

    if (device_type == CL_DEVICE_TYPE_DEFAULT || (dev->type & device_type)) {
      ++count;
      if (device_type == CL_DEVICE_TYPE_DEFAULT) break;  // DEFAULT: Returns only the first result.
    }
    dev = dev->next;
  }
  return count;
}

uint32_t dfcl_get_devices(cl_device_type device_type, cl_device_id *devices, uint32_t num_entries) {
  uint32_t dev_added = 0;
  _cl_device_id *dev = atomic_load(&dfcl_devices_list);

  while (dev != NULL && dev_added < num_entries) {
    if (dev->available == CL_FALSE) {
      dev = dev->next;
      continue;
    }

    if (device_type == CL_DEVICE_TYPE_DEFAULT || (dev->type & device_type)) {
      devices[dev_added++] = dev;
      if (device_type == CL_DEVICE_TYPE_DEFAULT) break;
    }
    dev = dev->next;
  }

  return dev_added;
}


cl_device_id *dfcl_unique_device_list(const cl_device_id *in, cl_uint num, cl_uint *real) {
  if (in == NULL || num == 0 || real == NULL) {
    *real = 0;
    return NULL;
  }

  /* Allocate output array (worst case: all devices are unique) */
  cl_device_id *out = (cl_device_id *)calloc(num, sizeof(cl_device_id));
  if (out == NULL) {
    *real = 0;
    return NULL;
  }

  cl_uint unique_count = 0;
  for (cl_uint i = 0; i < num; i++) {
    if (in[i] == NULL) continue;

    /* Get the real Root Device (follow parent chain) */
    cl_device_id root = in[i];
    while (root->parent_device != NULL) {
      root = root->parent_device;
    }

    /* Check if this root device is already in the list */
    bool found = false;
    for (cl_uint j = 0; j < unique_count; j++) {
      if (out[j] == root) {
        found = true;
        break;
      }
    }

    if (!found) {
      out[unique_count++] = root;
    }
  }

  *real = unique_count;
  return out;
}
