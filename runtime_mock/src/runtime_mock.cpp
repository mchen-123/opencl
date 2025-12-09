#include <vector>
#include <cstdlib>
#include <string>

#include "device.hpp"
#include "context.hpp"
#include "memory.hpp"
#include "runtime_mock.hpp"

DFResult dfInit(uint32_t flags) {
  return DF_SUCCESS;
}

DFResult dfDriverGetVersion(int *driverVersion) {
  *driverVersion = 1;
  return DF_SUCCESS;
}

DFResult dfDeviceGetCount(int *count) {
  auto devs = getAllDevices();
  *count = devs.size();
  return DF_SUCCESS;
}

DFResult dfDeviceGet(DFDevice *device, int devId) {
  auto devs = getAllDevices();
  auto dev = devs[devId].get();
  *device = reinterpret_cast<DFDevice>(dev);
  return DF_SUCCESS;
}

DFResult dfDeviceGetAttribute(int64_t *data, DFDeviceAttribute attribute, DFDevice dev) {
  *data = 42;
  return DF_SUCCESS;
}

DFResult dfCtxCreate(DFContext *pctx, uint32_t flags, DFDevice dev) {
    auto d = reinterpret_cast<Device*>(dev);
    auto ctx = userContextCreate(d);
    *pctx = reinterpret_cast<DFContext>(ctx);
    return DF_SUCCESS;
}

DFResult dfCtxDestroy(DFContext ctx) {
    userContextDestroy(reinterpret_cast<UserContext *>(ctx));
    return DF_SUCCESS;
}

DFResult dfStreamCreate(DFStream *phStream, uint32_t flags) {
  auto s = new Stream();
  *phStream = reinterpret_cast<DFStream>(s);
  return DF_SUCCESS;
}

DFResult dfCtxPushCurrent(DFContext ctx) {
  // int ret = contextPushCurrent(reinterpret_cast<Context *>(ctx));
  return DF_SUCCESS;
}

DFResult dfCtxPopCurrent(DFContext *pctx) {
  return DF_SUCCESS;
}
DFResult dfStreamDestroy(DFStream hStream) {
  return DF_SUCCESS;
}

DFResult dfStreamSynchronize(DFStream hStream) {
  return DF_SUCCESS;
}

DFResult dfMemAlloc(DFDeviceptr *dptr, const DFDieConfig *dieCfg, size_t sizePerDie) {
  auto m = new mem(10);
  *dptr = reinterpret_cast<DFDeviceptr>(m);
  return DF_SUCCESS;
}

DFResult dfMemFree(DFDeviceptr dptr) {
  return DF_SUCCESS;
}

DFResult dfMemcpyHtoD(DFDeviceptr dst, size_t offset, const DFDieConfig *dieCfg, void *src,
                      size_t sizePerDie, DFMemcpyFlag flags) {
  return DF_SUCCESS;
}

DFResult dfMemcpyDtoH(void *dst, DFDeviceptr src, size_t offset, const DFDieConfig *dieCfg,
                      size_t sizePerDie) {
  return DF_SUCCESS;
}

DFResult dfModuleLoad(DFModule *module, const char *fname) {
  return DF_SUCCESS;
}

DFResult dfModuleGetFunction(DFFunction *hfunc, DFModule hmod, const char *name) {
  return DF_SUCCESS;
}

DFResult dfModuleUnload(DFModule hmod) {
  return DF_SUCCESS;
}
DFResult dfLaunchKernel(DFFunction f, const DFDieConfig *dieCfg, uint32_t blocksPerDie,
                        uint32_t threadsPerBlock, uint64_t sharedMemSize, DFStream hStream,
                        void **kernelParams, void **extra) {
  return DF_SUCCESS;
}