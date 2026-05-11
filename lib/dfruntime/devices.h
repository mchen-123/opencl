#ifndef _DEVICES_H_
#define _DEVICES_H_

#include <CL/cl.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
* Initialize all devices for this platform (only self-developed GPUs).
 *
 * The initialization process includes:
 *   - Probing the number of available GPUs via dfcl_dfruntime_probe()
 *   - Allocating device structures
 *   - Initializing each device via dfcl_dfruntime_init()
 *   - Adding devices to the global device list
 *
 * \param platform The platform to which the devices belong.
 *
 * \return CL_SUCCESS if successful, otherwise an appropriate OpenCL error code.
 */
cl_int dfcl_init_devices(cl_platform_id platform);

/**
 * Return the total number of available self-developed GPUs in the system.
 *
 * This is a convenience function that returns the count of all initialized
 * and available devices (currently only self-developed Thrive GPUs).
 *
 * \return The number of available devices.
 */
cl_uint dfcl_get_device_count(void);

/**
 * Return the number of devices that match the specified device type.
 *
 * \param device_type The type of device to count.
 *
 * \return The number of matching devices.
 */
uint32_t dfcl_get_device_type_count(cl_device_type device_type);

/**
 * Get a list of devices matching the specified device type.
 *
 * This function fills the provided array with device IDs that match
 * the requested device_type. It skips unavailable devices.
 *
 * \param device_type  The type of devices to retrieve.
 * \param devices      [out] Pointer to an array where device IDs will be stored.
 * \param num_entries  Maximum number of devices that can be written to the array.
 *
 * \return The actual number of devices written to the devices array.
 */
uint32_t dfcl_get_devices(cl_device_type device_type, cl_device_id *devices, uint32_t num_entries);

#ifdef __cplusplus
}
#endif

#endif /* _DEVICES_H_ */