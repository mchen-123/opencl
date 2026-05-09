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

int dfcl_dfruntime_probe(struct dfcl_device_ops *ops)
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

cl_int dfcl_dfruntime_init(unsigned int dev_id, _cl_device_id *dev)
{
    DFResult result;
    cl_int ret = CL_SUCCESS;

    assert(dev->data == NULL);

    dev->vendor = "THRIVE Corporation";
    dev->vendor_id = 0x1234;
    dev->type = CL_DEVICE_TYPE_GPU;
    dev->address_bits = (sizeof(void *) * 8);
    dev->max_work_item_dimensions = 3;
    dev->image_support = CL_FALSE;
    dev->profile = "FULL_PROFILE";
    dev->parent_device = NULL;

    /* stream */
    dev->on_dev_queue_props = 0;
    dev->on_host_queue_props = CL_QUEUE_PROFILING_ENABLE;

    dfcl_dfruntime_device_data_t *data = 
        (dfcl_dfruntime_device_data_t *)calloc(1, sizeof(dfcl_dfruntime_device_data_t));

    result = dfDeviceGet(&data->device, dev_id);
    if (DFCA_CHECK_ERROR(result, "dfDeviceGet")) {
        free(data);
        return CL_INVALID_DEVICE;
    }

    dev->data = data;
    data->available = CL_TRUE;
    dev->available = CL_TRUE;
    
    dev->long_name = dev->short_name = "THRIVE H1s";
    dev->version = "H1s 0.3.0";

    /* 获取设备属性 */
    {
        uint32_t rc_acc = 0;
        int64_t tmp = 0;

        rc_acc |= dfDeviceGetAttribute(&tmp, DF_DEV_ATTR_PE_COUNT_IN_ONE_DIE, data->device);
        dev->max_compute_units = (cl_uint)tmp;
    }

    dev->single_fp_config = CL_FP_ROUND_TO_NEAREST | CL_FP_ROUND_TO_ZERO 
                          | CL_FP_ROUND_TO_INF | CL_FP_INF_NAN | CL_FP_FMA 
                          | CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT | CL_FP_DENORM;

    if (result != CL_INVALID_DEVICE) {
        result = dfCtxCreate(&data->context, 0, data->device);
        if (DFCA_CHECK_ERROR(result, "dfCtxCreate")) {
            free(data);
            return CL_INVALID_DEVICE;
        }
    }

    size_t free_ex_grid = 0, total_ex_grid = 0;
    DFDieMemInfo perDieInfos[1];

    if (result != CL_INVALID_DEVICE) {
        result = dfMemGetInfoEx(&free_ex_grid, &total_ex_grid, &singleDieCfg, perDieInfos);
        if (DFCA_CHECK_ERROR(result, "dfMemGetInfoEx")) {
            ret = CL_INVALID_DEVICE;
        } else {
            dev->die_max_mem_alloc_size = perDieInfos[0].free;
        }
    }

    return ret;
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

void dfcl_dfruntime_init_device_ops(struct dfcl_device_ops *ops)
{
    ops->device_name = "dfRuntime";

    ops->probe      = dfcl_dfruntime_probe;
    ops->init       = dfcl_dfruntime_init;
    ops->init_queue = dfcl_dfruntime_init_queue;
    ops->free_queue = dfcl_dfruntime_free_queue;
    ops->flush      = dfcl_dfruntime_flush;
}