#include "cl_device.hpp"
#include "devices.hpp"
#include "dfcl_cl.hpp"
#include <sstream>

#ifndef CL_USE_DEPRECATED_OPENCL_1_0_APIS
#define CL_USE_DEPRECATED_OPENCL_1_0_APIS
#endif

#ifndef CL_USE_DEPRECATED_OPENCL_1_1_APIS
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#endif

extern cl_platform_id damoPlatform;

extern "C" CL_API_ENTRY cl_int CL_API_CALL
clGetDeviceIDs(cl_platform_id platform,
                      cl_device_type device_type,
                      cl_uint num_entries,
                      cl_device_id* devices,
                      cl_uint* num_devices) CL_API_SUFFIX__VERSION_1_0 
{
    int total_num = 0;
    int devices_added = 0;
    if (platform == nullptr || platform != damoPlatform) {
        return CL_INVALID_PLATFORM;
    }

    int err = dfcl_init_devices(platform);
    if (err) 
        return err;

    total_num = dfcl_get_device_type_count(device_type);
    if (total_num == 0)
        return CL_DEVICE_NOT_FOUND;

    if (devices != nullptr) {
        devices_added = dfcl_get_devices(device_type, devices, num_entries);
    }

    if (num_devices != nullptr) {
        *num_devices = total_num;
    }

    if (devices_added > 0 || num_entries == 0) {
        return CL_SUCCESS;
    } else {
        return CL_DEVICE_NOT_FOUND;
    }
}

extern "C" CL_API_ENTRY cl_int CL_API_CALL
clGetDeviceInfo(cl_device_id device,
                    cl_device_info param_name,
                    size_t param_value_size,
                    void* param_value,
                    size_t* param_value_size_ret) 
{
    cl_int result = CL_SUCCESS;
    if (!is_valid(device))
        return CL_INVALID_DEVICE;

    switch (param_name) {
        case CL_DEVICE_IMAGE_SUPPORT:
        {
            CL_SET_RETURN_VAL(
                param_value_size, param_value, param_value_size_ret, cl_bool, device->image_support, result);
            break;
        }
        case CL_DEVICE_TYPE:
        {
            CL_SET_RETURN_VAL(
                param_value_size, param_value, param_value_size_ret, cl_device_type, device->type, result);
            break;
        }
        case CL_DEVICE_VENDOR_ID:
        {
            CL_SET_RETURN_VAL(
                param_value_size, param_value, param_value_size_ret, cl_uint, device->vendor_id, result);
            break;
        }
        case CL_DEVICE_MAX_COMPUTE_UNITS:
        {
            CL_SET_RETURN_VAL(
                param_value_size, param_value, param_value_size_ret, cl_uint, device->max_compute_units, result);
            break;
        }
        case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:
        {
            CL_SET_RETURN_VAL(
                param_value_size, param_value, param_value_size_ret, cl_uint, device->max_work_item_dimensions, result);
            break;
        }
        case CL_DEVICE_VENDOR:
        {
            const char *vendor = device->vendor;
            CL_SET_RETURN_PTR(
                param_value_size, param_value, param_value_size_ret, vendor, strlen(vendor) + 1, result);
            break;
        }
        case CL_DEVICE_NAME:
        {
            const char *name = device->long_name;
            CL_SET_RETURN_PTR(
                param_value_size, param_value, param_value_size_ret, name, strlen(name) + 1, result);
            break;
        }
        case CL_DEVICE_VERSION:
        {
            const char *version = device->version;
            CL_SET_RETURN_PTR(
                param_value_size, param_value, param_value_size_ret, version, strlen(version) + 1, result);
            break;
        }
        case CL_DEVICE_OPENCL_C_VERSION:
        {
            const char ch[] = "OpenCL C 3.0";
            CL_SET_RETURN_PTR(
                param_value_size, param_value, param_value_size_ret, ch, strlen(ch) + 1, result);
            break;
        }
        case CL_DEVICE_SINGLE_FP_CONFIG:
        {
            CL_SET_RETURN_VAL(
                param_value_size, param_value, param_value_size_ret, cl_device_fp_config, device->single_fp_config, result);
            break;
        }
        case CL_DEVICE_PROFILE:
        {
            const char *profile = device->profile;
            CL_SET_RETURN_PTR(
                param_value_size, param_value, param_value_size_ret, profile, strlen(profile) + 1, result);
            break;
        }
        case CL_DEVICE_ADDRESS_BITS:
        {
            CL_SET_RETURN_VAL(param_value_size, param_value, param_value_size_ret, cl_uint, device->address_bits, result);
            break;
        }
        case CL_DEVICE_PLATFORM:
        {
            CL_SET_RETURN_PTR(param_value_size, param_value, param_value_size_ret, &damoPlatform, sizeof(cl_platform_id), result);
            break;
        }
        default: result = CL_INVALID_VALUE;
    }
    return result;
}