#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

#include "cl_helper.h"

// atomic_int queue_c = {0};

// CL_API_ENTRY cl_command_queue CL_API_CALL
// clCreateCommandQueue(cl_context context,
//                      cl_device_id device,
//                      cl_command_queue_properties properties,
//                      cl_int* errcode_ret) CL_API_SUFFIX__VERSION_1_0
// {
//     cl_int errcode = CL_SUCCESS;
//     cl_uint i = 0;
//     cl_bool found = CL_FALSE;

//     if (!is_valid(context)) {
//         DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_CONTEXT, errcode_ret, NULL);
//     }
//     if (!is_valid(device) || (device->available != CL_TRUE)) {
//         DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_DEVICE, errcode_ret, NULL);
//     }

//     /* validate flags */
//     cl_command_queue_properties all_properties
//         = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE
//         | CL_QUEUE_ON_DEVICE | CL_QUEUE_ON_DEVICE_DEFAULT | CL_QUEUE_HIDDEN;

//     if ((properties & (~all_properties))) {
//         DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_VALUE, errcode_ret, NULL);
//     }
//     cl_command_queue_properties supported_device_props;
//     if (properties & (CL_QUEUE_ON_DEVICE | CL_QUEUE_ON_DEVICE_DEFAULT))
//         supported_device_props = device->on_dev_queue_props;
//     else
//         supported_device_props = device->on_host_queue_props | CL_QUEUE_HIDDEN;

//     if ((properties & supported_device_props) != properties) {
//         DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_QUEUE_PROPERTIES, errcode_ret, NULL);
//     }

//     for (i = 0; i < context->num_devices; ++i) {
//         if (context->devices[i] == device) {
//             found = CL_TRUE;
//         }
//     }
//     if (!found) {
//         DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_DEVICE, errcode_ret, NULL);
//     }

//     cl_command_queue command_queue = (cl_command_queue)malloc(sizeof(struct _cl_command_queue));
//     if (command_queue == NULL) {
//         DFCL_SET_FUNCTION_VALUE_RETURN(CL_OUT_OF_HOST_MEMORY, errcode_ret, NULL);
//     }

//     CL_INIT_OBJECT(command_queue, context);
//     atomic_init(&command_queue->refcount, 1);

//     command_queue->context = context;
//     command_queue->device = device;
//     command_queue->properties = properties;

//     /* hidden queues don't retain the context. */
//     if ((properties & CL_QUEUE_HIDDEN) == 0) {
//         dfcl_retain_object(context);
//         context_add_command_queue(context, command_queue);
//     }

//     errcode = CL_SUCCESS;
//     if (device->ops->init_queue)
//         errcode = device->ops->init_queue(device, command_queue);

//     DFCL_ATOMIC_INC(&queue_c);

//     if (errcode_ret != NULL)
//         *errcode_ret = errcode;

//     DFCL_MSG_INFO("Create Commandqueue: %p, RefCount: %d", command_queue,
//     dfcl_object_get_refcount((ThiveclObject *)command_queue));

//     return command_queue;
// }

// CL_API_ENTRY cl_int CL_API_CALL
// clFlush(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
// {
//     if (!is_valid(command_queue)) {
//         return CL_INVALID_COMMAND_QUEUE;
//     }

//     if (command_queue->device->available == CL_FALSE) {
//         return CL_DEVICE_NOT_AVAILABLE;
//     }

//     if (command_queue->device->ops->flush)
//         command_queue->device->ops->flush(command_queue->device, command_queue);
//     return CL_SUCCESS;
// }

CL_API_ENTRY cl_int CL_API_CALL clReleaseCommandQueue(cl_command_queue command_queue) {
  // if (!is_valid(command_queue)) {
  //     return CL_INVALID_COMMAND_QUEUE;
  // }

  // int new_refcount = dfcl_release_object(command_queue);
  // DFCL_MSG_INFO("Release Command Queue= %p, new_refcount =%d", command_queue, new_refcount);

  // if (new_refcount == 0) {

  //     cl_context context = command_queue->context;
  //     cl_device_id device = command_queue->device;

  //     DFCL_ATOMIC_DEC(&queue_c);
  //     if ((command_queue->properties & CL_QUEUE_HIDDEN) == 0) {
  //         context_remove_command_queue(context, command_queue);
  //         dfcl_release_object(context);
  //     }

  //     assert(command_queue->command_count == 0);

  //     if (command_queue->device->ops->free_queue
  //         && (command_queue->device->available) == CL_TRUE)
  //         command_queue->device->ops->free_queue(device, command_queue);

  //     DFCL_MEM_FREE(command_queue);
  // }

  return CL_SUCCESS;
}
