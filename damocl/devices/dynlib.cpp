#include "dynlib.hpp"

void *
dfcl_dynlib_open(const std::string& path, bool lazy, bool local) {
    int flags = 0;

    flags |= lazy ? RTLD_LAZY : RTLD_NOW;
    flags |= local ? RTLD_LOCAL : RTLD_GLOBAL;

    void* handle = dlopen(path.c_str(), flags);
    if (handle == nullptr) {
        const char* err_msg = dlerror();
        if (err_msg == nullptr) {
            std::cerr << "dlopen() failed without an error message" << std::endl;
        } else {
            std::cerr << "DFCL ERROR: " << err_msg << std::endl;
        }
    }
    return handle;
}

void *
dfcl_dynlib_symbol_address(void* dynlib_handle, const std::string& symbol_name) {
    void *addr = dlsym(dynlib_handle, symbol_name.c_str());
    if (addr == nullptr) {
        const char* err_msg = dlerror();
        if (err_msg == nullptr) {
            std::cerr << "dlsym() failed without an error message" << std::endl;
        } else {
            std::cerr << "DFCL ERROR: " << err_msg << std::endl;
        }
    }
    return addr;
}