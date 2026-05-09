#ifndef _DEVICES_H_
#define _DEVICES_H_

#include <CL/cl.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Populate the device with the wanted device types for a given platform.
 *
 * Should be called before accessing the device list. Can be called repeatedly.
 * The devices are shared across contexts, thus must implement resource
 * management internally also across multiple contexts.
 */
cl_int dfcl_init_devices(cl_platform_id platform);

/**
 * \brief Get the number of devices for a given device type.
 * \param device_type The type of devices to count.
 * \return The number of devices.
 */
uint32_t dfcl_get_device_type_count(cl_device_type device_type);

/**
 * \brief Get a list of devices for a given device type.
 * \param device_type The type of devices to get.
 * \param devices The array to store the device IDs.
 * \param num_entries Number of devices queried.
 * \return The real number of devices added to devices array which match the specified type
 */
uint32_t dfcl_get_devices(cl_device_type device_type, cl_device_id *devices, uint32_t num_entries);

#ifdef __cplusplus
}
#endif

#endif /* _DEVICES_H_ */