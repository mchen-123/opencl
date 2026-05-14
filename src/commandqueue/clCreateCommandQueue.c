#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

#include "cl_util.h"
#include "cl_helper.h"
#include "dfruntime/dfruntime.h"

atomic_int queue_c = {0};

CL_API_ENTRY cl_command_queue CL_API_CALL clCreateCommandQueue(
    cl_context context, cl_device_id device, cl_command_queue_properties properties,
    cl_int *errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  cl_int errcode = CL_SUCCESS;
  cl_bool found = CL_FALSE;
  cl_command_queue command_queue = NULL;

  DFCL_GOTO_ERROR_ON(!IS_VALID_OBJECT(context), CL_INVALID_CONTEXT, "invalid context %p", context);
  DFCL_GOTO_ERROR_ON(!IS_VALID_OBJECT(device) || (device->available != CL_TRUE), CL_INVALID_DEVICE,
                     "invalid device %p", device);

  // validate flags
  // 目前不支持任何额外属性（包括 Profiling 和 Out-of-Order)
  if (properties & ~(CL_QUEUE_HIDDEN)) {
    DFCL_GOTO_ERROR_ON(1, CL_INVALID_VALUE, "Only 0 (In-Order) or CL_QUEUE_HIDDEN are supported");
  }

  /* ==================== 检查设备是否属于 Context ==================== */
  for (uint32_t i = 0; i < context->num_devices; ++i) {
    if (context->devices[i] == dfcl_real_dev(device)) {
      found = CL_TRUE;
    }
  }
  DFCL_GOTO_ERROR_ON(!found, CL_INVALID_DEVICE, "Device %p is not in the context", device);

  /* ==================== 创建 Command Queue 对象 ==================== */
  command_queue = (cl_command_queue)DFCL_NEW(struct _cl_command_queue);
  DFCL_GOTO_ERROR_COND(command_queue == NULL, CL_OUT_OF_HOST_MEMORY);

  dfcl_init_object(command_queue, 0);

  command_queue->context = context;
  command_queue->device = device;
  command_queue->properties = properties;

  /* ==================== none default stream: retain Context 并加入链表 ==================== */
  if ((properties & CL_QUEUE_HIDDEN) == 0) {
    clRetainContext(context);
    DFCL_LOCK_OBJ(context);
    DL_APPEND(context->command_queues, command_queue);
    DFCL_UNLOCK_OBJ(context);
  }

  errcode = dfcl_dfruntime_init_queue(device, command_queue);
  DFCL_GOTO_ERROR_ON(errcode != CL_SUCCESS, errcode, "dfcl_dfruntime_init_queue failed with %d",
                     errcode);

  DFCL_ATOMIC_INC(&queue_c);

  if (errcode_ret) *errcode_ret = errcode;

  DFCL_MSG_INFO("Command Queue created: %p on device %p", command_queue, device);

  return command_queue;
ERROR:
  if (command_queue) DFCL_MEM_FREE(command_queue);

  if (errcode_ret) *errcode_ret = errcode;

  return NULL;
}
