#include <stdint.h>
#include <stddef.h>
#include "runtime_defines.hpp"

typedef enum {
  DF_SUCCESS,
  DF_INIT_FAIL,
} DFResult;

DFResult dfInit(uint32_t flags);

DFResult dfDriverGetVersion(int *driverVersion);

DFResult dfDeviceGetCount(int *count);

DFResult dfDeviceGet(DFDevice *device, int devId);

DFResult dfDeviceGetAttribute(int64_t *data, DFDeviceAttribute attribute, DFDevice dev);

DFResult dfCtxCreate(DFContext *pctx, uint32_t flags, DFDevice dev);

DFResult dfCtxDestroy(DFContext ctx);

DFResult dfStreamCreate(DFStream *phStream, uint32_t flags);

DFResult dfCtxPushCurrent(DFContext ctx);

DFResult dfStreamDestroy(DFStream hStream);

DFResult dfStreamSynchronize(DFStream hStream);

DFResult dfMemAlloc(DFDeviceptr *dptr, const DFDieConfig *dieCfg, size_t sizePerDie);

DFResult dfMemFree(DFDeviceptr dptr);

DFResult dfMemcpyHtoD(DFDeviceptr dst, size_t offset, const DFDieConfig *dieCfg, void *src,
                      size_t sizePerDie, DFMemcpyFlag flags);

DFResult dfMemcpyDtoH(void *dst, DFDeviceptr src, size_t offset, const DFDieConfig *dieCfg,
                      size_t sizePerDie);

DFResult dfMemFree(DFDeviceptr dptr);

DFResult dfModuleLoad(DFModule *module, const char *fname);

DFResult dfModuleUnload(DFModule hmod);

DFResult dfModuleGetFunction(DFFunction *hfunc, DFModule hmod, const char *name);

DFResult dfLaunchKernel(DFFunction f, const DFDieConfig *dieCfg, uint32_t blocksPerDie,
                        uint32_t threadsPerBlock, uint64_t sharedMemSize, DFStream hStream,
                        void **kernelParams, void **extra);