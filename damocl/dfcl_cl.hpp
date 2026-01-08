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
#include <list>

// Need to rename all CL API functions to prevent ICD loader functions calling
// themselves via the dispatch table. Include this before cl headers.
#include "cl_rename_api.hpp"

#include "runtime_mock.hpp"
#include "cl_icd_structs.hpp"
#include "cl_helper.hpp"
#include "cl_icd_structs.hpp"

/* dfcl specific flag, for "hidden" default queues allocated in each context */
#define CL_QUEUE_HIDDEN (1 << 10)

class DfclObject {
public:
    CL_OBJECT_BODY;
    mutable std::atomic<int> refcount{0};
    uint64_t id;
    mutable std::mutex mutex;

public:
    // 线程安全的引用计数操作
    int retain() const {
        return refcount.fetch_add(1, std::memory_order_relaxed);
    }

    int release() const {
        int old = refcount.fetch_sub(1, std::memory_order_acq_rel);
        return old - 1;
    }

    // 获取当前引用计数（用于调试）
    int getRefCount() const {
        return refcount.load(std::memory_order_relaxed);
    }

    // 锁守卫
    void lock() const   { mutex.lock(); }
    void unlock() const { mutex.unlock(); }

    // 方便的 RAII 锁
    class LockGuard {
    public:
        explicit LockGuard(const DfclObject* obj) : obj(obj) { obj->lock(); }
        ~LockGuard() { obj->unlock(); }
    private:
        const DfclObject* obj;
    };
};


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

struct _cl_device_id : public DfclObject
{
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

    /* OpenCL 2.0 properties */
    cl_command_queue_properties on_dev_queue_props;
    cl_command_queue_properties on_host_queue_props;

    std::atomic<_cl_device_id *> next = nullptr;
};

struct _cl_context : public DfclObject
{
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

    /* l*/
    std::list<cl_command_queue> command_queues;
};

struct _cl_command_queue  : public DfclObject {
    /* queries */
    cl_context context;
    cl_device_id device;
    cl_command_queue_properties properties;

    /* Number of unfinished command enqueued. */
    unsigned long command_count;
    /* device specific data */
    void *data;
};

struct dfcl_device_ops {
    const char *device_name;
    /**
     * Called when clFlush is called.
     *
     * This function ensures that
     * commands will be eventually executed. It is up to the device what happens
     * here, if anything. See basic and pthread for reference.*/
    void (*flush) (cl_device_id device, cl_command_queue cq);

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


    /** Optional: If the driver needs to use hardware resources
     * for command queues, it should use these callbacks */
    int (*init_queue) (cl_device_id device, cl_command_queue command_queue);
    int (*free_queue) (cl_device_id device, cl_command_queue queue);
};

template<typename T>
inline int dfcl_retain_object(T *obj) {
    if (!obj) return CL_INVALID_VALUE;
    return obj->retain();
}

template<typename T>
inline int dfcl_release_object(T *obj) {
    if (!obj) return CL_INVALID_VALUE;
    return obj->release();;
}

#endif /*_DFCL_CL_HPP_*/





