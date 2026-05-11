#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>

#include "devices.h"
#include "cl_helper.h"
#include "cl_util.h"
#include "dfcl-runtime.h"

/* ====================== 全局状态 ====================== */
static pthread_mutex_t g_devices_lock = PTHREAD_MUTEX_INITIALIZER;
static cl_bool         g_devices_initialized = CL_FALSE;

static int             g_num_devices = 0;
static _cl_device_id **g_all_devices = NULL;

static _Atomic(_cl_device_id *) dfcl_devices_list = NULL;

/* ====================== 链表辅助函数 ====================== */
static void ll_append_atomic(_cl_device_id *new_node)
{
    assert(new_node != NULL);
    new_node->next = NULL;

    _cl_device_id *last = atomic_load(&dfcl_devices_list);
    _cl_device_id *next = NULL;

    if (last == NULL) {
        if (atomic_compare_exchange_weak_explicit(&dfcl_devices_list,
                &last, new_node,
                memory_order_release, memory_order_relaxed)) {
            return;
        }
    }

    do {
        while ((next = atomic_load_explicit(&last->next, memory_order_acquire)) != NULL) {
            last = next;
        }
    } while (!atomic_compare_exchange_weak_explicit(&last->next,
                &next, new_node,
                memory_order_release, memory_order_relaxed));
}

/* ====================== Device Initialization ====================== */
cl_int dfcl_init_devices(cl_platform_id platform)
{
    if (g_devices_initialized)
        return CL_SUCCESS;

    pthread_mutex_lock(&g_devices_lock);
    if (g_devices_initialized) {
        pthread_mutex_unlock(&g_devices_lock);
        return CL_SUCCESS;
    }

    /* Probe Device Count */
    g_num_devices = dfcl_dfruntime_probe();
    if (g_num_devices <= 0) {
        pthread_mutex_unlock(&g_devices_lock);
        return CL_DEVICE_NOT_FOUND;
    }

    /* Allocate device pointer array */
    g_all_devices = calloc(g_num_devices, sizeof(_cl_device_id*));
    if (!g_all_devices) {
        g_num_devices = 0;
        pthread_mutex_unlock(&g_devices_lock);
        return CL_OUT_OF_HOST_MEMORY;
    }

    /* Create and initialize each device */
    for (int i = 0; i < g_num_devices; i++) {
        _cl_device_id *dev = calloc(1, sizeof(_cl_device_id));
        if (!dev) {
            goto error;
        }

        dev->dev_id         = i;
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
    pthread_mutex_unlock(&g_devices_lock);
    return CL_SUCCESS;

error:
    pthread_mutex_unlock(&g_devices_lock);
    return CL_DEVICE_NOT_FOUND;
}

/* ====================== Device Query ====================== */
uint32_t dfcl_get_device_type_count(cl_device_type device_type)
{
    uint32_t count = 0;
    _cl_device_id *dev = atomic_load(&dfcl_devices_list);

    while (dev != NULL) {
        if (dev->available == CL_FALSE) {
            dev = dev->next;
            continue;
        }

        if (device_type == CL_DEVICE_TYPE_DEFAULT || (dev->type & device_type)) {
            ++count;
            if (device_type == CL_DEVICE_TYPE_DEFAULT)
                break;                 // DEFAULT: Returns only the first result.
        }
        dev = dev->next;
    }
    return count;
}

uint32_t dfcl_get_devices(cl_device_type device_type,
                          cl_device_id *devices,
                          uint32_t num_entries)
{
    uint32_t dev_added = 0;
    _cl_device_id *dev = atomic_load(&dfcl_devices_list);

    while (dev != NULL && dev_added < num_entries) {
        if (dev->available == CL_FALSE) {
            dev = dev->next;
            continue;
        }

        if (device_type == CL_DEVICE_TYPE_DEFAULT || (dev->type & device_type)) {
            devices[dev_added++] = dev;
            if (device_type == CL_DEVICE_TYPE_DEFAULT)
                break;
        }
        dev = dev->next;
    }

    return dev_added;
}