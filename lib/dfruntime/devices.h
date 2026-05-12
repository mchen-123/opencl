#ifndef _DEVICES_H_
#define _DEVICES_H_

#include <CL/cl.h>
#include "../cl_helper.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct dfcl_dfruntime_device_data_s {
  DFDevice device;
  DFContext context;
  cl_bool available;
} dfcl_dfruntime_device_data_t;

#define DFCA_CHECK_ERROR(result, api) dfca_check_error(result, api, __FILE__, __LINE__)

/**
 * @brief Probe the DF runtime and return the number of available devices.
 *
 * Initializes the underlying runtime (dfInit) and queries device count.
 *
 * @return Number of detected devices, or 0 if none or initialization failed.
 */
int dfcl_dfruntime_probe(void);

/**
 * @brief Initialize one device with DF runtime.
 *
 * Sets OpenCL properties, detects Multi-Die support, and creates low-level context.
 *
 * @param dev_id  Device index
 * @param dev     Device structure to initialize
 * @return CL_SUCCESS on success
 */
cl_int dfcl_dfruntime_init(unsigned int dev_id, _cl_device_id *dev);

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

/**
 * @brief Deduplicate device list and convert Sub-Devices to Root Devices.
 *
 * Used by clCreateContext to ensure each physical device appears only once.
 *
 * @param[in]  in     Input array of device IDs (may contain duplicates and Sub-Devices)
 * @param[in]  num    Number of entries in input array
 * @param[out] real   Returns the actual number of unique Root Devices
 *
 * @return Allocated array of unique Root Devices, or NULL on error.
 * @note Caller must free the returned pointer.
 */
cl_device_id *dfcl_unique_device_list(const cl_device_id *in, cl_uint num, cl_uint *real);

#ifdef __cplusplus
}
#endif

#endif /* _DEVICES_H_ */
