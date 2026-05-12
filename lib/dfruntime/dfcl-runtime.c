// #include <CL/cl.h>
// #include <assert.h>
// #include <stdlib.h>
// #include <stdio.h>
// #include <stdint.h>
// #include <stdbool.h>

// #include "dfcl-runtime.h"
// #include "runtime_mock.h"

// typedef struct dfcl_dfruntime_queue_data_s {
//   DFStream stream;
//   cl_command_queue queue;
// } dfcl_dfruntime_queue_data_t;

// const DFDieConfig singleDieCfg = {.type = DF_DIE_CONFIG_TYPE_GRID, .grid = {{0, 0}, {0, 0}}};

// cl_int dfcl_dfruntime_init_queue(cl_device_id device, cl_command_queue queue) {
//   dfcl_dfruntime_device_data_t *dev_data = (dfcl_dfruntime_device_data_t *)queue->device->data;

//   dfCtxSetCurrent(dev_data->context);

//   dfcl_dfruntime_queue_data_t *queue_data =
//       (dfcl_dfruntime_queue_data_t *)calloc(1, sizeof(dfcl_dfruntime_queue_data_t));

//   queue->data = queue_data;
//   queue_data->queue = queue;

//   DFResult result = dfStreamCreate(&queue_data->stream, 0);
//   if (DFCA_CHECK_ERROR(result, "dfStreamCreate")) {
//     free(queue_data);
//     queue->data = NULL;
//     return CL_OUT_OF_RESOURCES;
//   }
//   return CL_SUCCESS;
// }

// int dfcl_dfruntime_free_queue(cl_device_id device, cl_command_queue cq) {
//   dfcl_dfruntime_queue_data_t *queue_data = (dfcl_dfruntime_queue_data_t *)cq->data;

//   dfcl_dfruntime_device_data_t *dev_data = (dfcl_dfruntime_device_data_t *)cq->device->data;

//   dfCtxSetCurrent(dev_data->context);

//   DFResult result = dfStreamDestroy(queue_data->stream);
//   if (DFCA_CHECK_ERROR(result, "dfStreamDestroy")) {
//     return CL_INVALID_COMMAND_QUEUE;
//   }

//   free(queue_data);
//   cq->data = NULL;

//   return CL_SUCCESS;
// }

// void dfcl_dfruntime_flush(cl_device_id device, cl_command_queue cq) {
//   /* TODO: Something here? */
// }
