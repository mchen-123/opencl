#pragma once
#ifndef RUNTIME_MOCK_H_
#define RUNTIME_MOCK_H_

#include <stdint.h>
#include <stddef.h>
#include "runtime_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DF_SUCCESS,
    DF_INIT_FAIL,
    DF_ERROR,
    DF_ERROR_UNKNOWN
} DFResult;

/* Error handling */
DFResult dfGetErrorName(DFResult result, const char **name);
DFResult dfGetErrorString(DFResult result, const char **name);

/* Initialization */
DFResult dfInit(uint32_t flags);
DFResult dfDriverGetVersion(int *driverVersion);

/* Device management */
DFResult dfDeviceGetCount(int *count);
DFResult dfDeviceGet(DFDevice *device, int devId);
DFResult dfDeviceGetAttribute(int64_t *data, DFDeviceAttribute attribute, DFDevice dev);

/* Context */
DFResult dfCtxCreate(DFContext *pctx, uint32_t flags, DFDevice dev);
DFResult dfCtxDestroy(DFContext ctx);
DFResult dfCtxSetCurrent(DFContext ctx);
DFResult dfCtxPushCurrent(DFContext ctx);

/* Stream */
DFResult dfStreamCreate(DFStream *phStream, uint32_t flags);
DFResult dfStreamDestroy(DFStream hStream);
DFResult dfStreamSynchronize(DFStream hStream);

/* Memory */
DFResult dfMemAlloc(DFDeviceptr *dptr, const DFDieConfig *dieCfg, size_t sizePerDie);
DFResult dfMemFree(DFDeviceptr dptr);

DFResult dfMemGetInfoEx(size_t *freeMem, size_t *totalMem, const DFDieConfig *dieCfg, DFDieMemInfo *perDieInfos);

DFResult dfMemcpyHtoD(DFDeviceptr dst, size_t offset, const DFDieConfig *dieCfg,
                      void *src, size_t sizePerDie, DFMemcpyFlag flags);

DFResult dfMemcpyDtoH(void *dst, DFDeviceptr src, size_t offset,
                      const DFDieConfig *dieCfg, size_t sizePerDie);

/* Module & Kernel */
DFResult dfModuleLoad(DFModule *module, const char *fname);
DFResult dfModuleUnload(DFModule hmod);
DFResult dfModuleGetFunction(DFFunction *hfunc, DFModule hmod, const char *name);

DFResult dfLaunchKernel(DFFunction f, const DFDieConfig *dieCfg,
                        uint32_t blocksPerDie, uint32_t threadsPerBlock,
                        uint64_t sharedMemSize, DFStream hStream,
                        void **kernelParams, void **extra);

DFResult dfDeviceGetDieGrid(DFDevice device, DFDieGrid *grid);

#ifdef __cplusplus
}
#endif

#endif /* RUNTIME_MOCK_H_ */