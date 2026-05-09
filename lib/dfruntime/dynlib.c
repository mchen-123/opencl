#include "dynlib.h"
#include <stdio.h>
#include <string.h>

void *
dfcl_dynlib_open(const char *path, int lazy, int local) {
    int flags = 0;

    flags |= lazy ? RTLD_LAZY : RTLD_NOW;
    flags |= local ? RTLD_LOCAL : RTLD_GLOBAL;

    void *handle = dlopen(path, flags);
    if (handle == NULL) {
        const char *err_msg = dlerror();
        if (err_msg == NULL) {
            fprintf(stderr, "dlopen() failed without an error message\n");
        } else {
            fprintf(stderr, "DFCL ERROR: %s\n", err_msg);
        }
    }
    return handle;
}

void *
dfcl_dynlib_symbol_address(void *dynlib_handle, const char *symbol_name) {
    void *addr = dlsym(dynlib_handle, symbol_name);
    if (addr == NULL) {
        const char *err_msg = dlerror();
        if (err_msg == NULL) {
            fprintf(stderr, "dlsym() failed without an error message\n");
        } else {
            fprintf(stderr, "DFCL ERROR: %s\n", err_msg);
        }
    }
    return addr;
}