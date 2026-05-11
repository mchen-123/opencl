#include <CL/cl.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "dfcl-runtime.h"
#include "runtime_mock.h"

typedef struct dfcl_dfruntime_device_data_s {
    DFDevice device;
    DFContext context;
    cl_bool available;
} dfcl_dfruntime_device_data_t;

typedef struct dfcl_dfruntime_queue_data_s {
    DFStream stream;
    cl_command_queue queue;
} dfcl_dfruntime_queue_data_t;

const DFDieConfig singleDieCfg = {.type = DF_DIE_CONFIG_TYPE_GRID, .grid = {{0, 0}, {0, 0}}};

static inline bool dfca_check_error(DFResult result, const char *api, const char *file, int line)
{
    if (result == DF_SUCCESS)
        return false;

    const char *err_name = NULL;
    const char *err_msg = NULL;
    dfGetErrorName(result, &err_name);
    dfGetErrorString(result, &err_msg);

    fprintf(stderr, "[%s:%d] Runtime error during %s: %s(%s)\n", 
            file, line, api, 
            err_name ? err_name : "NULL", 
            err_msg ? err_msg : "NULL");
    
    return true;
}

#define DFCA_CHECK_ERROR(result, api) dfca_check_error(result, api, __FILE__, __LINE__)

int dfcl_dfruntime_probe(void)
{
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

/**
 * Initialize a single device with low-level DFRuntime.
 *
 * This function sets basic OpenCL properties and queries
 * Multi-Die information from the underlying driver.
 */
cl_int dfcl_dfruntime_init(unsigned int dev_id, _cl_device_id *dev)
{
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
    dev->vendor             = "THRIVE Corporation";
    dev->vendor_id          = 0x1234;
    dev->type               = CL_DEVICE_TYPE_GPU;
    dev->address_bits       = (sizeof(void *) * 8);
    dev->max_work_item_dimensions = 3;
    dev->image_support      = CL_FALSE;
    dev->profile            = "FULL_PROFILE";
    dev->parent_device      = NULL;

    dev->long_name = dev->short_name = "THRIVE H1s";
    dev->version   = "OpenCL 3.0 THRIVE 1.0";
    dev->driver_version = "0.6.0";

    /* Single precision floating point capabilities */
    dev->single_fp_config = CL_FP_ROUND_TO_NEAREST | CL_FP_ROUND_TO_ZERO 
                          | CL_FP_ROUND_TO_INF | CL_FP_INF_NAN | CL_FP_FMA 
                          | CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT | CL_FP_DENORM;

    /* ====================== Multi-Die Information ====================== */
    DFDieGrid grid = {{0, 0}, {0, 0}};
    result = dfDeviceGetDieGrid(data->device, &grid);
    if (result == DF_SUCCESS && (grid.bottomRight.x > 0 || grid.bottomRight.y > 0)) {
        dev->is_multi_die = true;
        dev-> max_compute_units = dev->num_dies = (grid.bottomRight.x + 1) * (grid.bottomRight.y + 1);
        dev->die_grid     = grid;

        DFCL_MSG_INFO("Device %d: Multi-Die enabled, grid=(%d,%d)-(%d,%d), total Dies=%u",
                      dev_id,
                      grid.upperLeft.x, grid.upperLeft.y,
                      grid.bottomRight.x, grid.bottomRight.y,
                      dev->num_dies);
    } else {
        dev->is_multi_die = false;
        dev->num_dies     = 1;
        DFCL_MSG_INFO("Device %d: Single Die mode", dev_id);
    }

    /* ====================== Create Context ====================== */
    result = dfCtxCreate(&data->context, 0, data->device);
    if (DFCA_CHECK_ERROR(result, "dfCtxCreate")) {
        free(data);
        return CL_INVALID_DEVICE;
    }

    /* Attach private data */
    dev->data       = data;
    data->available = CL_TRUE;
    dev->available  = CL_TRUE;

    DFCL_MSG_INFO("Device %d initialized successfully (Multi-Die: %s)", 
                  dev_id, dev->is_multi_die ? "Yes" : "No");
    return CL_SUCCESS;
}

cl_int dfcl_dfruntime_init_queue(cl_device_id device, cl_command_queue queue)
{
    dfcl_dfruntime_device_data_t *dev_data = 
        (dfcl_dfruntime_device_data_t *)queue->device->data;

    dfCtxSetCurrent(dev_data->context);

    dfcl_dfruntime_queue_data_t *queue_data = 
        (dfcl_dfruntime_queue_data_t *)calloc(1, sizeof(dfcl_dfruntime_queue_data_t));

    queue->data = queue_data;
    queue_data->queue = queue;

    DFResult result = dfStreamCreate(&queue_data->stream, 0);
    if (DFCA_CHECK_ERROR(result, "dfStreamCreate")) {
        free(queue_data);
        queue->data = NULL;
        return CL_OUT_OF_RESOURCES;
    }
    return CL_SUCCESS;
}

int dfcl_dfruntime_free_queue(cl_device_id device, cl_command_queue cq)
{
    dfcl_dfruntime_queue_data_t *queue_data = 
        (dfcl_dfruntime_queue_data_t *)cq->data;

    dfcl_dfruntime_device_data_t *dev_data = 
        (dfcl_dfruntime_device_data_t *)cq->device->data;

    dfCtxSetCurrent(dev_data->context);

    DFResult result = dfStreamDestroy(queue_data->stream);
    if (DFCA_CHECK_ERROR(result, "dfStreamDestroy")) {
        return CL_INVALID_COMMAND_QUEUE;
    }

    free(queue_data);
    cq->data = NULL;

    return CL_SUCCESS;
}

void dfcl_dfruntime_flush(cl_device_id device, cl_command_queue cq)
{
    /* TODO: Something here? */
}