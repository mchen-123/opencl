#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <stdatomic.h>
#include <limits.h>
#include <libgen.h>

#include "devices.h"
#include "cl_helper.h"
#include "cl_util.h"
#include "dynlib.h"

static pthread_mutex_t dfcl_init_lock = PTHREAD_MUTEX_INITIALIZER;

int dfcl_offline_compile = 0;

static int first_init_done = 0;
static int init_in_progress = 0;
uint64_t dfcl_num_devices = 0;

/* Head for the dfcl_devices linked list */
static _Atomic(_cl_device_id *) dfcl_devices_list = {0};

typedef void (*init_device_ops)(struct dfcl_device_ops *);
static init_device_ops dfcl_devices_init_ops;

static struct dfcl_device_ops dfcl_device_ops_t;
#define DFCL_DEVICE_LIB "libdfcl-device.so"
static void *dfcl_device_handle = NULL;

static uint64_t device_count;

/* Indexes each device added to the platform by setting the device id. First
 * used and modified during init, to index devices present since launch. May
 * also used and modified when devices are dynamically added. */
static uint64_t dev_index;

static void ll_append_atomic(_cl_device_id *new_node) {
    assert(new_node != NULL);

    new_node->next = NULL; /* new node next pointer is NULL */

    _cl_device_id *head = atomic_load(&dfcl_devices_list);
    _cl_device_id *last = head;
    _cl_device_id *next;

    if (last == NULL) {
        /* if head is NULL, try to set it to new_node */
        if (atomic_compare_exchange_weak_explicit(&dfcl_devices_list, &last, new_node, memory_order_release, memory_order_relaxed)) {
            return;
        }
    }

    do {
        while ((next = atomic_load_explicit(&last->next, memory_order_acquire)) != NULL) {
            last = next;
        }
    } while (!atomic_compare_exchange_weak_explicit(&last->next, &next, new_node, memory_order_release, memory_order_acquire));
}

static const char *get_dfcl_device_lib_path(void) {
    static char path_buf[PATH_MAX];
    Dl_info info;

    if (dladdr((void *)get_dfcl_device_lib_path, &info) && info.dli_fname) {
        char *dir = dirname((char *)info.dli_fname);
        snprintf(path_buf, sizeof(path_buf), "%s/%s", dir, DFCL_DEVICE_LIB);
        return path_buf;
    }

    return DFCL_DEVICE_LIB;
}

cl_int
dfcl_init_devices(cl_platform_id platform) {
    int errcode = CL_SUCCESS;

    if (init_in_progress) {
        return errcode;
    }

    pthread_mutex_lock(&dfcl_init_lock);
    init_in_progress = 1;

    if (first_init_done) {
        /* 第二次及以后：直接返回当前状态; todo: 需要重新探测设备 */
        init_in_progress = 0;
        pthread_mutex_unlock(&dfcl_init_lock);
        return dfcl_num_devices > 0 ? CL_SUCCESS : CL_DEVICE_NOT_FOUND;
    }

    dfcl_offline_compile = dfcl_get_bool_option("OFFLINE_COMPILE", 0);

    const char *deviceLibrary = get_dfcl_device_lib_path();
    dfcl_device_handle = dfcl_dynlib_open(deviceLibrary, 1, 0);
    if (dfcl_device_handle == NULL) {
        DFCL_MSG_ERR("Loading %s failed", deviceLibrary);
        init_in_progress = 0;
        pthread_mutex_unlock(&dfcl_init_lock);
        return CL_DEVICE_NOT_FOUND;
    }
    DFCL_MSG_INFO("Fallback Loaded %s succeeded", deviceLibrary);

    const char *init_device_ops_name = "dfcl_dfruntime_init_device_ops";
    dfcl_devices_init_ops = (init_device_ops)dfcl_dynlib_symbol_address(
        dfcl_device_handle, init_device_ops_name);
    if (!dfcl_devices_init_ops) {
        DFCL_MSG_ERR("Loading symbol %s from %s failed", init_device_ops_name, deviceLibrary);
        init_in_progress = 0;
        pthread_mutex_unlock(&dfcl_init_lock);
        return CL_DEVICE_NOT_FOUND;
    }

    dfcl_devices_init_ops(&dfcl_device_ops_t);
    assert(dfcl_device_ops_t.device_name != NULL);

    /* Probe and add the result to the number of probed devices */
    assert(dfcl_device_ops_t.probe != NULL);
    device_count = dfcl_device_ops_t.probe(&dfcl_device_ops_t);
    dfcl_num_devices = device_count;

    if (device_count == 0) {
        DFCL_MSG_ERR("No device found");
        errcode = CL_DEVICE_NOT_FOUND;
        goto ERROR;
    }

    dev_index = 0;

    {
        int i;
        for(i = 0; i < (int)device_count; ++i) {
            _cl_device_id *dev = (_cl_device_id *)malloc(sizeof(_cl_device_id));
            if (dev == NULL) {
                errcode = CL_OUT_OF_HOST_MEMORY;
                goto ERROR;
            }
            memset(dev, 0, sizeof(_cl_device_id));

            dev->ops = &dfcl_device_ops_t;
            dev->dev_id = dev_index;
            CL_INIT_OBJECT(dev, platform);
            atomic_init(&dev->refcount, 1);
            dev->next = NULL;
            dev->driver_version = "0.3.0";
            if (dev->version == NULL) {
                dev->version = "OpenCL 3.0 THRIVE 1.0";
            }
            errcode = dev->ops->init(i, dev);
            if (errcode != CL_SUCCESS) {
                DFCL_MSG_ERR("Device init failed");
                free(dev);
                goto ERROR;
            }

            ll_append_atomic(dev);
            ++dev_index;
        }
    }
    first_init_done = 1;
ERROR:
    init_in_progress = 0;
    pthread_mutex_unlock(&dfcl_init_lock);
    return errcode;
}

uint32_t
dfcl_get_device_type_count(cl_device_type device_type) {
    uint32_t count = 0;
    _cl_device_id *dev;

    if (device_type == CL_DEVICE_TYPE_DEFAULT) {
        dev = (_cl_device_id *)atomic_load(&dfcl_devices_list);
        return dev ? 1 : 0;
    }

    dev = (_cl_device_id *)atomic_load(&dfcl_devices_list);
    while (dev != NULL) {
        if (!dfcl_offline_compile && (dev->available == CL_FALSE)) {
            dev = dev->next;
            continue;
        }

        if (dev->type & device_type) {
            ++count;
        }
        dev = dev->next;
    }
    return count;
}

uint32_t
dfcl_get_devices(cl_device_type device_type, cl_device_id *devices, uint32_t num_entries) {
    uint32_t dev_added = 0;
    _cl_device_id *dev;

    dev = (_cl_device_id *)atomic_load(&dfcl_devices_list);
    while (dev != NULL) {
        if (!dfcl_offline_compile && (dev->available == CL_FALSE)) {
            dev = dev->next;
            continue;
        }

        if (device_type == CL_DEVICE_TYPE_DEFAULT) {
            devices[dev_added] = dev;
            ++dev_added;
            break;
        }

        if (dev->type & device_type) {
            if (dev_added < num_entries) {
                devices[dev_added] = dev;
                ++dev_added;
            } else {
                break;
            }
        }
        dev = dev->next;
    }

    return dev_added;
}