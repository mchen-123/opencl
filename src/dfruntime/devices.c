#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>
#include <stdbool.h>

#include "devices.h"
#include "dynlib.h"
#include "cl_helper.h"
#include "cl_util.h"

/* ====================== Global Runtime Ops ====================== */
dfcl_runtime_ops_t g_runtime_ops = {0};

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
  if (g_runtime_ops.dfGetErrorName) g_runtime_ops.dfGetErrorName(result, &err_name);
  if (g_runtime_ops.dfGetErrorString) g_runtime_ops.dfGetErrorString(result, &err_msg);

  fprintf(stderr, "[%s:%d] Runtime error during %s: %s(%s)\n", file, line, api,
          err_name ? err_name : "NULL", err_msg ? err_msg : "NULL");

  return true;
}

/* ====================== Runtime Library Loading ====================== */
cl_int dfcl_load_runtime_library(void) {
  /* Already loaded? */
  if (g_runtime_ops.handle != NULL) return CL_SUCCESS;

  const char *lib_path = "/opt/thrive/lib/libruntime_mock.so";

  void *handle = dfcl_dynlib_open(lib_path, 1, 0);
  if (handle == NULL) {
    fprintf(stderr, "[DFCL] Failed to load runtime library: %s\n", lib_path);
    return CL_INVALID_OPERATION;
  }

  /* Resolve all symbols */
#define DFCL_LOAD_SYM(name)                                                   \
  do {                                                                        \
    g_runtime_ops.name = (name##_t)dfcl_dynlib_symbol_address(handle, #name); \
    if (g_runtime_ops.name == NULL) {                                         \
      fprintf(stderr, "[DFCL] Failed to resolve symbol: %s\n", #name);        \
      dlclose(handle);                                                        \
      return CL_INVALID_OPERATION;                                            \
    }                                                                         \
  } while (0)

  DFCL_LOAD_SYM(dfInit);
  DFCL_LOAD_SYM(dfDriverGetVersion);
  DFCL_LOAD_SYM(dfDeviceGetCount);
  DFCL_LOAD_SYM(dfDeviceGet);
  DFCL_LOAD_SYM(dfDeviceGetAttribute);
  DFCL_LOAD_SYM(dfCtxCreate);
  DFCL_LOAD_SYM(dfCtxDestroy);
  DFCL_LOAD_SYM(dfCtxSetCurrent);
  DFCL_LOAD_SYM(dfCtxPushCurrent);
  DFCL_LOAD_SYM(dfStreamCreate);
  DFCL_LOAD_SYM(dfStreamDestroy);
  DFCL_LOAD_SYM(dfStreamSynchronize);
  DFCL_LOAD_SYM(dfMemAlloc);
  DFCL_LOAD_SYM(dfMemFree);
  DFCL_LOAD_SYM(dfMemcpyHtoD);
  DFCL_LOAD_SYM(dfMemcpyDtoH);
  DFCL_LOAD_SYM(dfMemGetInfoEx);
  DFCL_LOAD_SYM(dfModuleLoad);
  DFCL_LOAD_SYM(dfModuleUnload);
  DFCL_LOAD_SYM(dfModuleGetFunction);
  DFCL_LOAD_SYM(dfLaunchKernel);
  DFCL_LOAD_SYM(dfDeviceGetDieGrid);
  DFCL_LOAD_SYM(dfGetErrorName);
  DFCL_LOAD_SYM(dfGetErrorString);

#undef DFCL_LOAD_SYM

  g_runtime_ops.handle = handle;
  DFCL_MSG_INFO("Runtime library loaded successfully: %s\n", lib_path);
  return CL_SUCCESS;
}

/* ====================== Device Probe & Init ====================== */
int dfcl_dfruntime_probe(void) {
  int probe_count = 0;
  DFResult ret = g_runtime_ops.dfInit(0);
  if (ret == DF_SUCCESS) {
    ret = g_runtime_ops.dfDeviceGetCount(&probe_count);
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
  dfcl_dfruntime_device_data_t *data = DFCL_NEW(dfcl_dfruntime_device_data_t);
  if (data == NULL) {
    return CL_OUT_OF_HOST_MEMORY;
  }

  result = g_runtime_ops.dfDeviceGet(&data->device, dev_id);
  if (DFCA_CHECK_ERROR(result, "dfDeviceGet")) {
    DFCL_MEM_FREE(data);
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
  result = g_runtime_ops.dfDeviceGetDieGrid(data->device, &grid);
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
  result = g_runtime_ops.dfCtxCreate(&data->context, 0, data->device);
  if (DFCA_CHECK_ERROR(result, "dfCtxCreate")) {
    DFCL_MEM_FREE(data);
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
  cl_int errcode = CL_SUCCESS;

  if (g_devices_initialized) return CL_SUCCESS;

  DFCL_LOCK(g_devices_lock);
  if (g_devices_initialized) {
    DFCL_UNLOCK(g_devices_lock);
    return CL_SUCCESS;
  }

  /* Step 1: Load the runtime library dynamically */
  errcode = dfcl_load_runtime_library();
  if (errcode != CL_SUCCESS) {
    DFCL_UNLOCK(g_devices_lock);
    return errcode;
  }

  /* Step 2: Probe Device Count */
  g_num_devices = dfcl_dfruntime_probe();
  if (g_num_devices <= 0) {
    DFCL_UNLOCK(g_devices_lock);
    return CL_DEVICE_NOT_FOUND;
  }

  /* Step 3: Allocate device pointer array */
  g_all_devices = DFCL_NEW_ARRAY(_cl_device_id *, g_num_devices);
  if (!g_all_devices) {
    g_num_devices = 0;
    DFCL_UNLOCK(g_devices_lock);
    return CL_OUT_OF_HOST_MEMORY;
  }

  /* Step 4: Create and initialize each device */
  for (int i = 0; i < g_num_devices; i++) {
    _cl_device_id *dev = DFCL_NEW(_cl_device_id);
    DFCL_GOTO_ERROR_COND(!dev, CL_OUT_OF_HOST_MEMORY);

    dfcl_init_object(dev, i);  // id 从 1 开始递增

    dev->dev_id = i;
    errcode = dfcl_dfruntime_init(i, dev);
    DFCL_GOTO_ERROR_ON(errcode != CL_SUCCESS, errcode,
                       "dfcl_dfruntime_init failed for device %d with %d", i, errcode);

    g_all_devices[i] = dev;
    ll_append_atomic(dev);
  }

  g_devices_initialized = CL_TRUE;
  DFCL_MSG_INFO("Successfully initialized %d self-developed GPU(s)", g_num_devices);
  DFCL_UNLOCK(g_devices_lock);
  return CL_SUCCESS;

ERROR:
  /* 清理已创建的设备 */
  DFCL_MEM_FREE(g_all_devices);
  g_num_devices = 0;

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
  cl_device_id *out = DFCL_NEW_ARRAY(cl_device_id, num);
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

/* For a subdevice parameter, return the actual device it belongs to. */
cl_device_id dfcl_real_dev(const cl_device_id dev) {
  cl_device_id ret = dev;
  while (ret->parent_device) ret = ret->parent_device;
  return ret;
}

/**
 * @brief Initialize a Command Queue for DF runtime.
 *
 * Currently uses simple synchronous mode (no submit thread).
 */
cl_int dfcl_dfruntime_init_queue(cl_device_id device, cl_command_queue queue) {
  /* 1. 参数检查 */
  if (device == NULL || queue == NULL) return CL_INVALID_VALUE;

  dfcl_dfruntime_device_data_t *dev_data = (dfcl_dfruntime_device_data_t *)device->data;
  if (dev_data == NULL || dev_data->context == NULL) return CL_INVALID_DEVICE;

  /* 2. 切换到当前设备的 Context */
  DFResult ret = g_runtime_ops.dfCtxSetCurrent(dev_data->context);
  if (DFCA_CHECK_ERROR(ret, "dfCtxSetCurrent")) return CL_INVALID_DEVICE;

  /* 3. 分配队列私有数据 */
  dfcl_dfruntime_queue_data_t *queue_data = DFCL_NEW(dfcl_dfruntime_queue_data_t);
  if (queue_data == NULL) return CL_OUT_OF_HOST_MEMORY;

  queue->data = queue_data;
  queue_data->queue = queue;
  queue_data->device = device;

  /* 4. 创建底层 Stream（这是最核心的操作） */
  ret = g_runtime_ops.dfStreamCreate(&queue_data->stream, 0);
  if (DFCA_CHECK_ERROR(ret, "dfStreamCreate")) {
    DFCL_MEM_FREE(queue_data);
    return CL_OUT_OF_RESOURCES;
  }

  DFCL_MSG_INFO("Command Queue %p initialized successfully on device %d (stream=%p)", queue,
                device->dev_id, queue_data->stream);

  return CL_SUCCESS;
}

/**
 * @brief Free a Command Queue's DF runtime resources.
 *
 * Destroys the underlying stream and releases queue private data.
 *
 * @param device  The device this queue belongs to
 * @param queue   The command queue to free
 * @return CL_SUCCESS on success, or an OpenCL error code
 */
cl_int dfcl_dfruntime_free_queue(cl_device_id device, cl_command_queue queue) {
  if (device == NULL || queue == NULL) return CL_INVALID_VALUE;

  dfcl_dfruntime_queue_data_t *queue_data = (dfcl_dfruntime_queue_data_t *)queue->data;
  if (queue_data == NULL) return CL_SUCCESS;

  dfcl_dfruntime_device_data_t *dev_data = (dfcl_dfruntime_device_data_t *)device->data;
  if (dev_data == NULL || dev_data->context == NULL) return CL_INVALID_DEVICE;

  /* Switch to the device's context */
  DFResult ret = g_runtime_ops.dfCtxSetCurrent(dev_data->context);
  if (DFCA_CHECK_ERROR(ret, "dfCtxSetCurrent")) return CL_INVALID_DEVICE;

  /* Destroy the underlying stream */
  ret = g_runtime_ops.dfStreamDestroy(queue_data->stream);
  if (DFCA_CHECK_ERROR(ret, "dfStreamDestroy")) {
    return CL_INVALID_COMMAND_QUEUE;
  }

  DFCL_MSG_INFO("Command Queue %p freed on device %d (stream=%p destroyed)", queue, device->dev_id,
                queue_data->stream);

  DFCL_MEM_FREE(queue_data);

  return CL_SUCCESS;
}
