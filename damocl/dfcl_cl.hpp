#pragma once

#ifndef _DFCL_CL_HPP_
#define _DFCL_CL_HPP_

#include <CL/cl.h>
#include <CL/cl_icd.h>
#include <memory>
#include <atomic>
#include <cstring>
#include <iostream>

// Need to rename all CL API functions to prevent ICD loader functions calling
// themselves via the dispatch table. Include this before cl headers.
#include "cl_rename_api.hpp"

#include "runtime_mock.hpp"
#include "cl_icd_structs.hpp"
#include "cl_helper.hpp"
#include "cl_icd_structs.hpp"

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

    /* queries */
    cl_device_type type;
    cl_uint vendor_id;
    cl_uint max_compute_units;

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

#endif /*_DFCL_CL_HPP_*/





