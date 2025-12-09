#pragma once
#ifndef _CL_COMMAND_HPP_
#define _CL_COMMAND_HPP_

#include "cl_object.hpp"

class OpenclCommandqueue : public OpenclObject {
public:
    using cl_type = cl_command_queue;
    
    explicit OpenclCommandqueue(DFStream s) : stream_(s){};

    OpenclCommandqueue(const OpenclCommandqueue&) = delete;
    OpenclCommandqueue& operator=(const OpenclCommandqueue&) = delete;

    DFStream get() const { return stream_; }

    cl_uint retain() { return ++refcount_; }
    
    ~OpenclCommandqueue() {
        destroyCommandqueue();
    }

    cl_uint release() {
        cl_uint new_cnt = --refcount_;
        if (new_cnt == 0) {
            delete this;
        }
        return new_cnt;
    }

    void destroyCommandqueue() {
        if (stream_) {
            dfStreamDestroy(stream_);
            stream_ = nullptr;
        }
    }
    virtual ObjectType type() const override { return OBJECT_TYPE_COMMAND_QUEUE; }
public:
    DFStream stream_{nullptr};
    std::atomic<cl_uint> refcount_{1};
};

inline OpenclCommandqueue* as_runtime(cl_command_queue cl_handle) {
    if (!cl_handle) return nullptr;
    return ICDDispatchedObject::fromHandle<OpenclCommandqueue>(cl_handle);
}

#endif /*_CL_COMMAND_HPP_*/