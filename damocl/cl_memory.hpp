#pragma once
#ifndef _CL_MEMORY_HPP_
#define _CL_MEMORY_HPP_

#include <CL/cl.h>

#include "cl_object.hpp"

class OpenclMemory : public OpenclObject {
public:
    using cl_type = cl_mem;

    explicit OpenclMemory(DFDeviceptr dptr) : dptr_(dptr) {};

    DFDeviceptr get() const { return dptr_; }

    DFDeviceptr* getPointer() { return &dptr_; }

    cl_uint retain() { return ++refcount_; }

    ~OpenclMemory() {
        destroyMemory();
    }

    cl_uint release() {
        cl_uint new_cnt = --refcount_;
        if (new_cnt == 0) {
            delete this;
        }
        return new_cnt;
    }

    void destroyMemory() {
        if (dptr_) {
            dfMemFree(dptr_);
            dptr_ = nullptr;
        }
    }

    virtual ObjectType type() const override { return OBJECT_TYPE_MEM_OBJ; }
private:
    DFDeviceptr dptr_;
    std::atomic<cl_uint> refcount_{1};
};

inline OpenclMemory* as_internal(cl_mem cl_handle) {
    if (!cl_handle) return nullptr;
    return ICDDispatchedObject::fromHandle<OpenclMemory>(cl_handle);
}

#endif /*_CL_MEMORY_HPP_*/