#ifndef _DYNLIB_H_
#define _DYNLIB_H_

#include <dlfcn.h>

#ifdef __cplusplus
extern "C" {
#endif

void *dfcl_dynlib_open(const char *path, int lazy, int local);

void *dfcl_dynlib_symbol_address(void *dynlib_handle, const char *symbol_name);

#ifdef __cplusplus
}
#endif

#endif /* _DYNLIB_H_ */