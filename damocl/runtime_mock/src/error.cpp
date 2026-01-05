#include "runtime_mock.hpp"

DFResult dfGetErrorName(DFResult result, const char **name) {
    switch (result) {
        case DF_SUCCESS: *name = "DF_SUCCESS"; break;
        case DF_INIT_FAIL: *name = "DF_INIT_FAIL"; break;
        case DF_ERROR: *name = "DF_ERROR"; break;
        case DF_ERROR_UNKNOWN: *name = "DF_ERROR_UNKNOWN"; break;
        default: break;
    }
    return DF_SUCCESS;
}

DFResult dfGetErrorString(DFResult result, const char **name) {
    switch (result) {
        case DF_SUCCESS: *name = "success"; break;
        case DF_INIT_FAIL: *name = "initialization error"; break;
        case DF_ERROR: *name = "error"; break;
        case DF_ERROR_UNKNOWN: *name = "unknown error"; break;
        default: break;
    }
    return DF_SUCCESS;
}