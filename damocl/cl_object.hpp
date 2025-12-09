#ifndef _CL_OBJECT_HPP_
#define _CL_OBJECT_HPP_

#include <CL/cl.h>
#include <CL/cl_icd.h>
#include <memory>
#include <atomic>
#include <cstring>
#include <iostream>

// Need to rename all CL API functions to prevent ICD loader functions calling
// themselves via the dispatch table. Include this before cl headers.
#include "cl_rename_api.hpp"

#include "runtime_mock.hpp"
#include "cl_icd_structs.hpp"
#include "cl_helper.hpp"

extern CLIicdDispatchTable *g_icd_dispatchTable;

class ICDDispatchedObject {
protected:
    const CLIicdDispatchTable* const dispatch_{nullptr};
    ICDDispatchedObject() : dispatch_(g_icd_dispatchTable) {}

public:

    void* handle() { return static_cast<ICDDispatchedObject*>(this); }

    template <typename T> static T* fromHandle(void* handle) {
        return static_cast<T*>(reinterpret_cast<ICDDispatchedObject*>(handle));
    }
    template <typename T> static const T* fromHandle(const void* handle) {
        return static_cast<const T*>(reinterpret_cast<const ICDDispatchedObject*>(handle));
    }
};

/// @brief raw pointer → cl_xxx
template<typename T>
inline auto as_cl(T* obj) noexcept {
    static_assert(std::is_base_of_v<ICDDispatchedObject, T>,
        "T must inherit from ICDDispatchedObject");
    return obj ? static_cast<typename T::cl_type>(obj->handle()) : nullptr;
}

/// @brief shared_ptr → cl_xxx
template<typename T>
inline auto as_cl(const std::shared_ptr<T>& obj) noexcept {
    static_assert(std::is_base_of_v<ICDDispatchedObject, T>,
        "T must inherit from ICDDispatchedObject");
    return obj ? static_cast<typename T::cl_type>(obj->handle()) : nullptr;
}

class OpenclObject : public ICDDispatchedObject {
public:
    enum ObjectType {
        OBJECT_TYPE_CONTEXT = 0,
        OBJECT_TYPE_DEVICE,
        OBJECT_TYPE_MEM_OBJ,
        OBJECT_TYPE_KERNEL,
        OBJECT_TYPE_COMMAND_QUEUE,
        OBJECT_TYPE_PROGRAM,
        OBJECT_TYPE_SAMPLER,
        OBJECT_TYPE_EVENT,
    };
    virtual ObjectType type() const = 0;

    virtual ~OpenclObject() = default;
};

#endif // CL_OBJECT_HPP

