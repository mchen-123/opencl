#pragma once

#ifndef _CL_CONTEXT_HPP_
#define _CL_CONTEXT_HPP_

#include "cl_object.hpp"

class OpenclContext : public OpenclObject {
public:
    using cl_type = cl_context;

    explicit OpenclContext(DFContext* ctx, cl_uint numDevs, [[maybe_unused]] cl_uint totalDevs) 
                    : contexts_(ctx), num_devices_(numDevs), total_platform_devices_() {};
    OpenclContext(const OpenclContext&) = delete;
    OpenclContext& operator=(const OpenclContext&) = delete;
    
    cl_uint getNumDevices() const { return num_devices_; }

    DFContext* getContexts() const { return contexts_; }

    virtual ObjectType type() const override { return OBJECT_TYPE_CONTEXT; }

    cl_uint retain() { return ++refcount_;}
    
    ~OpenclContext() {
        destroyContext();
    }

    cl_uint release() {
        cl_uint new_cnt = --refcount_;
        if (new_cnt == 0) {
            delete this;
        }
        return new_cnt;
    }

    void destroyContext() {
        if (contexts_) {
            for (cl_uint i = 0; i < num_devices_; ++i) {
                if (contexts_[i]) dfCtxDestroy(contexts_[i]);
            }
            DF_DELETE_ARRAY(contexts_);
            contexts_ = nullptr;
        }
    }
private:
    DFContext* contexts_{nullptr};
    cl_uint num_devices_ {0};
    cl_int total_platform_devices_ {0};
    std::atomic<cl_uint> refcount_{1};
};

inline OpenclContext* as_runtime(cl_context cl_handle) {
    if (!cl_handle) return nullptr;
    return ICDDispatchedObject::fromHandle<OpenclContext>(cl_handle);
}

#endif /*_CL_CONTEXT_HPP_*/