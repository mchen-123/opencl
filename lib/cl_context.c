#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include "cl_util.h"
#include "cl_helper.h"

dfcl_lock_t dfcl_context_handling_lock = PTHREAD_MUTEX_INITIALIZER;

CL_API_ENTRY cl_context CL_API_CALL clCreateContext(
    const cl_context_properties *properties, cl_uint num_devices, const cl_device_id *devices,
    void(CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *), void *user_data,
    cl_int *errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  cl_int errcode = CL_SUCCESS;
  cl_uint i = 0;
  cl_context context = NULL;

  DFCL_LOCK(dfcl_context_handling_lock);
  /* ==================== Parameter Validation ==================== */
  if (devices == NULL || num_devices == 0) {
    errcode = CL_INVALID_VALUE;
    goto ERROR;
  }
  if (pfn_notify == NULL && user_data != NULL) {
    errcode = CL_INVALID_VALUE;
    goto ERROR;
  }

  /* ====================== 初始化设备 ====================== */
  cl_platform_id platform;
  clGetPlatformIDs(1, &platform, NULL);
  errcode = dfcl_init_devices(platform);
  if (errcode != CL_SUCCESS) {
    if (errcode == CL_DEVICE_NOT_FOUND) errcode = CL_INVALID_DEVICE;
    goto ERROR;
  }

  /* ==================== 创建 Context 对象 ==================== */
  context = (cl_context)calloc(1, sizeof(struct _cl_context));
  if (context == NULL) {
    errcode = CL_OUT_OF_HOST_MEMORY;
    goto ERROR;
  }

  CL_INIT_OBJECT(context, devices[0]);
  atomic_init(&context->refcount, 1);

  /* ====================== 保存设备列表 ====================== */
  context->create_devices = calloc(num_devices, sizeof(cl_device_id));
  if (context->create_devices == NULL) {
    errcode = CL_OUT_OF_HOST_MEMORY;
    goto ERROR;
  }
  memcpy(context->create_devices, devices, num_devices * sizeof(cl_device_id));
  context->num_create_devices = num_devices;

  context->devices = dfcl_unique_device_list(devices, num_devices, &context->num_devices);
  if (context->devices == NULL) {
    errcode = CL_OUT_OF_HOST_MEMORY;
    goto ERROR;
  }
  if (context->num_devices == 0) {
    errcode = CL_INVALID_DEVICE;
    goto ERROR;
  }

  /* ====================== 创建默认 Command Queue ====================== */
  context->default_queues = (cl_command_queue *)calloc(num_devices, sizeof(cl_command_queue));
  if (context->default_queues == NULL) {
    errcode = CL_OUT_OF_HOST_MEMORY;
    goto ERROR;
  }

  for (i = 0; i < context->num_devices; i++) {
    cl_device_id dev = context->devices[i];

    if (dev->available != CL_TRUE) {
      errcode = CL_INVALID_DEVICE;
      goto ERROR;
    }
  }

  /* ====================== 增加设备引用计数 ====================== */
  for (i = 0; i < context->num_create_devices; i++) {
    clRetainDevice(context->create_devices[i]);
  }
  /* ====================== 9. 完成创建并返回 ====================== */
  if (errcode_ret) *errcode_ret = CL_SUCCESS;

  DFCL_MSG_INFO("Created Context %p with %u devices", context, context->num_devices);

  DFCL_UNLOCK(dfcl_context_handling_lock);
  return context;
ERROR:
  if (context) {
    if (context->default_queues) {
      // for (i = 0; i < context->num_devices; i++) {
      //     if (context->default_queues[i])
      //         clReleaseCommandQueue(context->default_queues[i]);
      // }
      free(context->default_queues);
    }
    free(context->devices);
    free(context->create_devices);
    free(context->properties);
    free(context);
  }

  if (errcode_ret) *errcode_ret = errcode;
  DFCL_UNLOCK(dfcl_context_handling_lock);
  return NULL;
}

// CL_API_ENTRY cl_int CL_API_CALL
// clRetainContext(cl_context context) CL_API_SUFFIX__VERSION_1_0
// {
//     if (!is_valid(context)) {
//         return CL_INVALID_CONTEXT;
//     }

//     dfcl_retain_object(context);
//     return CL_SUCCESS;
// }

// CL_API_ENTRY cl_int CL_API_CALL
// clReleaseContext(cl_context context) CL_API_SUFFIX__VERSION_1_0
// {
//     if (!is_valid(context)) {
//         return CL_INVALID_CONTEXT;
//     }

//     cl_uint i = 0;
//     int new_refcount = dfcl_release_object(context);
//     DFCL_MSG_INFO("Release Context: %p, new_refcount =%d", context, new_refcount);

//     if (new_refcount == 0) {

//         DFCL_ATOMIC_DEC(&context_c);

//         for (i = 0; i < context->num_devices; i++) {
//             /* cl_device_id dev = context->devices[i]; */
//             if (context->default_queues && context->default_queues[i]) {
//                 /* todo: release default queue */
//             }
//         }

//         for (i = 0; i < context->num_create_devices; i++) {
//             /* todo: +releaseDevice(context->create_devices[i]); */
//         }

//         DFCL_MEM_FREE(context->create_devices);
//         DFCL_MEM_FREE(context->default_queues);
//         /* DFCL_MEM_FREE(context->devices); */
//         DFCL_MEM_FREE(context->properties);

//         DFCL_MEM_FREE(context);
//         --cl_context_count;
//     }

//     return CL_SUCCESS;
// }
