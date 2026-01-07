#pragma once

#ifndef _DFCL_CL_HPP_
#define _DFCL_CL_HPP_

#include <CL/cl.h>
#include <CL/cl_icd.h>
#include <memory>
#include <atomic>
#include <cstring>
#include <iostream>
#include <mutex>
#include <cassert>

// Need to rename all CL API functions to prevent ICD loader functions calling
// themselves via the dispatch table. Include this before cl headers.
#include "cl_rename_api.hpp"

#include "runtime_mock.hpp"
#include "cl_icd_structs.hpp"
#include "cl_helper.hpp"
#include "cl_icd_structs.hpp"

#define CL_OBJECT_COMMON_FIELDS \
    std::atomic<int> refcount{0}; \
    mutable std::mutex mutex

struct _cl_platform_id
{
    CL_OBJECT_BODY;
    const char *profile;
    const char *version;
    const char *name;
    const char *vendor;
    const char *extensions;
    const char *suffix;
};

struct _cl_device_id
{
    CL_OBJECT_BODY;
    CL_OBJECT_COMMON_FIELDS;
    /* queries */
    cl_device_type type;
    cl_uint vendor_id;
    cl_uint max_compute_units;

    /* for subdevice support */
    cl_device_id parent_device;

    cl_uint max_work_item_dimensions;
    cl_uint address_bits;
    cl_device_fp_config single_fp_config;

    /* Device specific operations, shared among devices of the same type */
    struct dfcl_device_ops *ops;
    
    /* Device ID within the device type */
    int dev_id;

    const char *short_name;
    const char *long_name;

    const char *vendor;
    const char *driver_version;
    const char *profile;

    /* these are Device versions, not OpenCL C versions */
    const char *version;

    cl_bool image_support;

    cl_bool available;

    /* Device specific data needed for internal devicee functions */
    void *data;

    std::atomic<_cl_device_id *> next = nullptr;
};

struct _cl_context
{
    CL_OBJECT_BODY;
    CL_OBJECT_COMMON_FIELDS;
    /* queries */
    cl_device_id *devices;
    cl_context_properties *properties;

    /* implementation */
    uint32_t num_devices;
    uint32_t num_properties;
    
    /* the original device list given to clCreateContext,
     * required for */
    cl_device_id *create_devices;
    uint32_t num_create_devices;

    /* for enqueueing migration commands. Two reasons:
     * 1) since migration commands can execute in parallel
     * to other commands, we can increase parallelism
     * 2) in some cases (migration between 2 devices through
     * host memory), we need to put two commands in two queues,
     * and the clEnqueueX only gives us one (on the destination
     * device). */
    cl_command_queue *default_queues;
};

struct dfcl_device_ops {
    const char *device_name;

    /**
     * Detects & returns the number of available devices the driver finds on the
     * system. 
     */
    int (*probe)(struct dfcl_device_ops *ops);

    /**
     * Initializes a device.
     * @param i The device index
     * @param device The cl_device_id to initialize
     */
    cl_int (*init)(unsigned int i, _cl_device_id *device);

};

template<typename T>
inline int dfcl_retain_object(T *obj) {
    if (!obj) return 0;
    return obj->refcount.fetch_add(1, std::memory_order_relaxed);
}

template<typename T>
inline int dfcl_release_object(T *obj) {
    if (!obj) return 0;
    return obj->refcount.fetch_sub(1, std::memory_order_release);
}

template<typename T>
class ClObjectLockGuard {
public:
    explicit ClObjectLockGuard(const T *obj) : guard_(obj->mutex) {
        assert(obj->refcount.load(std::memory_order_relaxed) > 0);
    }
private:
    std::lock_guard<std::mutex> guard_;
};

#endif /*_DFCL_CL_HPP_*/





