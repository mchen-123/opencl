#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include "cl_util.h"
#include "cl_helper.h"
#include "dfruntime/dfruntime.h"

dfcl_lock_t dfcl_context_handling_lock = PTHREAD_MUTEX_INITIALIZER;
atomic_int context_c = {0};

CL_API_ENTRY cl_context CL_API_CALL clCreateContext(
    const cl_context_properties *properties, cl_uint num_devices, const cl_device_id *devices,
    void(CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *), void *user_data,
    cl_int *errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  /* Suppress unused parameter warning */
  (void)properties;

  cl_int errcode = CL_SUCCESS;
  cl_context context = NULL;

  DFCL_LOCK(dfcl_context_handling_lock);
  /* ==================== Parameter Validation ==================== */
  DFCL_GOTO_ERROR_ON(devices == NULL || num_devices == 0, CL_INVALID_VALUE,
                     "devices is NULL or num_devices is 0");
  DFCL_GOTO_ERROR_ON(pfn_notify == NULL && user_data != NULL, CL_INVALID_VALUE,
                     "pfn_notify is NULL but user_data is not NULL");

  /* ====================== 初始化设备 ====================== */
  cl_platform_id platform;
  clGetPlatformIDs(1, &platform, NULL);
  errcode = dfcl_init_devices(platform);
  DFCL_GOTO_ERROR_ON(errcode != CL_SUCCESS,
                     errcode == CL_DEVICE_NOT_FOUND ? CL_INVALID_DEVICE : errcode,
                     "dfcl_init_devices failed with %d", errcode);

  /* ==================== 创建 Context 对象 ==================== */
  context = (cl_context)DFCL_NEW(struct _cl_context);
  DFCL_GOTO_ERROR_COND(context == NULL, CL_OUT_OF_HOST_MEMORY);

  dfcl_init_object(context, 0);  // id = 0 表示自动生成或后续再设置

  /* ====================== 保存设备列表 ====================== */
  context->create_devices = DFCL_NEW_ARRAY(cl_device_id, num_devices);
  DFCL_GOTO_ERROR_COND(context->create_devices == NULL, CL_OUT_OF_HOST_MEMORY);
  memcpy(context->create_devices, devices, num_devices * sizeof(cl_device_id));
  context->num_create_devices = num_devices;

  context->devices = dfcl_unique_device_list(devices, num_devices, &context->num_devices);
  DFCL_GOTO_ERROR_ON(context->devices == NULL || context->num_devices == 0, CL_INVALID_DEVICE,
                     "dfcl_unique_device_list returned empty list");

  /* ====================== 创建默认 Command Queue ====================== */
  context->default_queues = DFCL_NEW_ARRAY(cl_command_queue, context->num_devices);
  DFCL_GOTO_ERROR_COND(context->default_queues == NULL, CL_OUT_OF_HOST_MEMORY);
  for (cl_uint i = 0; i < context->num_devices; i++) {
    cl_device_id dev = context->devices[i];
    DFCL_GOTO_ERROR_ON(dev->available != CL_TRUE, CL_INVALID_DEVICE, "device[%u] is not available",
                       i);

    /* Create hidden default command queue for each device */
    cl_int qerr;
    cl_command_queue_properties props = CL_QUEUE_HIDDEN;
    context->default_queues[i] = clCreateCommandQueue(context, dev, props, &qerr);
    DFCL_GOTO_ERROR_ON(qerr != CL_SUCCESS, CL_INVALID_CONTEXT,
                       "failed to create default command queue for device[%u]: %d", i, qerr);
  }

  /* ====================== 增加设备引用计数 ====================== */
  for (cl_uint i = 0; i < context->num_create_devices; i++) {
    clRetainDevice(context->create_devices[i]);
  }
  /* ====================== 完成创建并返回 ====================== */
  DFCL_ATOMIC_INC(&context_c);

  if (errcode_ret) *errcode_ret = CL_SUCCESS;

  DFCL_MSG_INFO("Created Context %p with %u devices", context, context->num_devices);

  DFCL_UNLOCK(dfcl_context_handling_lock);
  return context;
ERROR:
  if (context) {
    if (context->default_queues) {
      for (cl_uint i = 0; i < context->num_devices; i++) {
        if (context->default_queues[i]) {
          clReleaseCommandQueue(context->default_queues[i]);
        }
      }
      DFCL_MEM_FREE(context->default_queues);
    }
    DFCL_MEM_FREE(context->devices);
    DFCL_MEM_FREE(context->create_devices);
    DFCL_MEM_FREE(context->properties);
    DFCL_MEM_FREE(context);
  }

  if (errcode_ret) *errcode_ret = errcode;
  DFCL_UNLOCK(dfcl_context_handling_lock);
  return NULL;
}
