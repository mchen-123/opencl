#include <string>
#include <dlfcn.h>
#include <iostream>
#include <CL/cl.h>

#ifdef __cplusplus
extern "C" {
#endif

void *dfcl_dynlib_open(const std::string& path, bool lazy, bool local);

void *dfcl_dynlib_symbol_address(void* dynlib_handle, const std::string& symbol_name);

#ifdef __cplusplus
}
#endif
