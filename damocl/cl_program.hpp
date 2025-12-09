#pragma once

#ifndef _CL_PROGRAM_HPP_
#define _CL_PROGRAM_HPP_

#include <vector>

#include "cl_object.hpp"
#include "cl_context.hpp"
#include "cl_memory.hpp"

#define KERNEL_EFL_FILE "kernel_add_f32_no_vpu.dfcafb"

class OpenclProgram : public OpenclObject {
public:
    using cl_type = cl_program;
    
    explicit OpenclProgram(OpenclContext* ctx) : ctx_(ctx) {};

    OpenclProgram(const OpenclProgram&) = delete;
    OpenclProgram& operator=(const OpenclProgram&) = delete;

    DFModule getModule() const { return module_; }

    void setModule(DFModule hmod) { module_ = hmod; }

    ~OpenclProgram() {
        destroyModule();
    }

    cl_uint retain() { return ++refcount_; }

    cl_uint release() {
        cl_uint new_cnt = --refcount_;
        if (new_cnt == 0) {
            delete this;
        }
        return new_cnt;
    }

    void destroyModule() {
        if (module_) {
            dfModuleUnload(module_);
            module_ = nullptr;
        }
    }

    virtual ObjectType type() const override { return OBJECT_TYPE_MEM_OBJ; }
private:
    OpenclContext* ctx_;
    DFModule module_;
    std::atomic<cl_uint> refcount_{1};
};

inline OpenclProgram* as_runtime(cl_program cl_handle) {
    if (!cl_handle) return nullptr;
    return ICDDispatchedObject::fromHandle<OpenclProgram>(cl_handle);
}

class kernelParameters {
public:
    struct Arg {
        enum class Kind { MEM, POD, LOCAL } kind;
        size_t size;
        void* value;

        Arg(OpenclMemory* mem)         : kind(Kind::MEM), size(sizeof(OpenclMemory)), value(mem) {}
        Arg(size_t sz, void* p)     : kind(Kind::POD), size(sz), value(p) {}
        Arg(size_t sz)              : kind(Kind::LOCAL), size(sz), value(nullptr) {}
        
        ~Arg() { 
            if (kind == Kind::POD) { 
                ::operator delete(value); 
            }
        }
    };

    kernelParameters() = default;
    
    void add_mem(OpenclMemory* mem) {
        args_.emplace_back(mem);
    }

    template<typename T>
    void add(const T& val) {
        static_assert(std::is_trivially_copyable<T>::value, "Only trivial types are supported");
        void* ptr = ::operator new(sizeof(T));
        std::memcpy(ptr, &val, sizeof(T));
        args_.emplace_back(sizeof(T), ptr);
    }

    void clear() { args_.clear(); }
    size_t size() const { return args_.size(); }
    const Arg& operator[](size_t i) const { return args_[i]; }

private:
    std::vector<Arg> args_;
};

class OpenclKernel : public OpenclObject {
public:
    using cl_type = cl_kernel;
    explicit OpenclKernel(OpenclProgram* program, const char* kernel_name, DFFunction func) 
        : program_(program), kernel_name_(kernel_name), kernel_func_(func), parameters_(new kernelParameters()) {};
    
    kernelParameters* parameters() { return parameters_; }

    DFFunction getFunction() const { return kernel_func_; }

    cl_uint retain() { return ++refcount_; }

    cl_uint release() {
        cl_uint new_cnt = --refcount_;
        if (new_cnt == 0) {
            delete this;
        }
        return new_cnt;
    }

    virtual ObjectType type() const override { return OBJECT_TYPE_KERNEL; }
private:
    OpenclProgram* program_;
    const char* kernel_name_;
    DFFunction kernel_func_;
    kernelParameters* parameters_;
    std::atomic<cl_uint> refcount_{1};
};

inline OpenclKernel* as_runtime(cl_kernel cl_handle) {
    if (!cl_handle) return nullptr;
    return ICDDispatchedObject::fromHandle<OpenclKernel>(cl_handle);
}

#endif /*_CL_PROGRAM_HPP_*/