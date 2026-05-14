#ifndef _DEVICES_H_
#define _DEVICES_H_

#include <CL/cl.h>
#include "../cl_helper.h"
#ifdef __cplusplus
extern "C" {
#endif

/* ==================== Runtime Function Pointer Types ==================== */
typedef DFResult (*dfInit_t)(uint32_t flags);
typedef DFResult (*dfDriverGetVersion_t)(int *driverVersion);
typedef DFResult (*dfDeviceGetCount_t)(int *count);
typedef DFResult (*dfDeviceGet_t)(DFDevice *device, int devId);
typedef DFResult (*dfDeviceGetAttribute_t)(int64_t *data, DFDeviceAttribute attribute,
                                           DFDevice dev);
typedef DFResult (*dfCtxCreate_t)(DFContext *pctx, uint32_t flags, DFDevice dev);
typedef DFResult (*dfCtxDestroy_t)(DFContext ctx);
typedef DFResult (*dfCtxSetCurrent_t)(DFContext ctx);
typedef DFResult (*dfCtxPushCurrent_t)(DFContext ctx);
typedef DFResult (*dfStreamCreate_t)(DFStream *phStream, uint32_t flags);
typedef DFResult (*dfStreamDestroy_t)(DFStream hStream);
typedef DFResult (*dfStreamSynchronize_t)(DFStream hStream);
typedef DFResult (*dfMemAlloc_t)(DFDeviceptr *dptr, const DFDieConfig *dieCfg, size_t sizePerDie);
typedef DFResult (*dfMemFree_t)(DFDeviceptr dptr);
typedef DFResult (*dfMemcpyHtoD_t)(DFDeviceptr dst, size_t offset, const DFDieConfig *dieCfg,
                                   void *src, size_t sizePerDie, DFMemcpyFlag flags);
typedef DFResult (*dfMemcpyDtoH_t)(void *dst, DFDeviceptr src, size_t offset,
                                   const DFDieConfig *dieCfg, size_t sizePerDie);
typedef DFResult (*dfMemGetInfoEx_t)(size_t *freeMem, size_t *totalMem, const DFDieConfig *dieCfg,
                                     DFDieMemInfo *perDieInfos);
typedef DFResult (*dfModuleLoad_t)(DFModule *module, const char *fname);
typedef DFResult (*dfModuleUnload_t)(DFModule hmod);
typedef DFResult (*dfModuleGetFunction_t)(DFFunction *hfunc, DFModule hmod, const char *name);
typedef DFResult (*dfLaunchKernel_t)(DFFunction f, const DFDieConfig *dieCfg, uint32_t blocksPerDie,
                                     uint32_t threadsPerBlock, uint64_t sharedMemSize,
                                     DFStream hStream, void **kernelParams, void **extra);
typedef DFResult (*dfDeviceGetDieGrid_t)(DFDevice device, DFDieGrid *grid);
typedef DFResult (*dfGetErrorName_t)(DFResult result, const char **name);
typedef DFResult (*dfGetErrorString_t)(DFResult result, const char **name);

/* ==================== Runtime Ops Table (like pocl_device_ops) ==================== */
typedef struct dfcl_runtime_ops_s {
  void *handle; /* dlopen handle */

  dfInit_t dfInit;
  dfDriverGetVersion_t dfDriverGetVersion;
  dfDeviceGetCount_t dfDeviceGetCount;
  dfDeviceGet_t dfDeviceGet;
  dfDeviceGetAttribute_t dfDeviceGetAttribute;
  dfCtxCreate_t dfCtxCreate;
  dfCtxDestroy_t dfCtxDestroy;
  dfCtxSetCurrent_t dfCtxSetCurrent;
  dfCtxPushCurrent_t dfCtxPushCurrent;
  dfStreamCreate_t dfStreamCreate;
  dfStreamDestroy_t dfStreamDestroy;
  dfStreamSynchronize_t dfStreamSynchronize;
  dfMemAlloc_t dfMemAlloc;
  dfMemFree_t dfMemFree;
  dfMemcpyHtoD_t dfMemcpyHtoD;
  dfMemcpyDtoH_t dfMemcpyDtoH;
  dfMemGetInfoEx_t dfMemGetInfoEx;
  dfModuleLoad_t dfModuleLoad;
  dfModuleUnload_t dfModuleUnload;
  dfModuleGetFunction_t dfModuleGetFunction;
  dfLaunchKernel_t dfLaunchKernel;
  dfDeviceGetDieGrid_t dfDeviceGetDieGrid;
  dfGetErrorName_t dfGetErrorName;
  dfGetErrorString_t dfGetErrorString;
} dfcl_runtime_ops_t;

/* Global runtime ops instance — initialized by dfcl_load_runtime_library() */
extern dfcl_runtime_ops_t g_runtime_ops;

typedef struct dfcl_dfruntime_device_data_s {
  DFDevice device;
  DFContext context;
  cl_bool available;
} dfcl_dfruntime_device_data_t;

typedef struct dfcl_dfruntime_queue_data_s {
  DFStream stream;
  cl_command_queue queue;
  cl_device_id device;
} dfcl_dfruntime_queue_data_t;

#define DFCA_CHECK_ERROR(result, api) dfca_check_error(result, api, __FILE__, __LINE__)

/**
 * @brief Load the runtime library from /opt/thrive/lib/libruntime_mock.so
 *        and resolve all function pointers into g_runtime_ops.
 *
 * Must be called once before any df* functions are used.
 *
 * @return CL_SUCCESS on success, or an OpenCL error code.
 */
cl_int dfcl_load_runtime_library(void);

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

cl_int dfcl_dfruntime_init_queue(cl_device_id device, cl_command_queue queue);

cl_int dfcl_dfruntime_free_queue(cl_device_id device, cl_command_queue queue);

cl_device_id dfcl_real_dev(const cl_device_id dev);
#ifdef __cplusplus
}
#endif

#endif /* _DEVICES_H_ */
