#include "cl_memory.hpp"

extern "C" CL_API_ENTRY cl_mem CL_API_CALL
clCreateBuffer(cl_context context, cl_mem_flags flags, size_t size, void* host_ptr, cl_int* errcode_ret) {
    cl_int result = CL_SUCCESS;
    cl_mem mem = nullptr;
    DFDeviceptr dptr = nullptr;

    // todo: Define dies to use. pe dies.
    DFDieCoord upperLeft = {0, 0};
    DFDieCoord bottomRight = {0, 0};  // Example coordinates for two dies
    DFDieGrid dieGrid = {upperLeft, bottomRight};
    DFDieConfig dieConfig;
    dieConfig.type = DF_DIE_CONFIG_TYPE_GRID;
    dieConfig.grid = dieGrid;

    if (!is_valid(context)) {
        DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_CONTEXT, errcode_ret, mem)
    }
    if (flags &
        ~(CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR | CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY |
             CL_MEM_READ_WRITE | CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS)) {
        DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_VALUE, errcode_ret, mem)
    }
    if (!is_valid(host_ptr) && (flags & (CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR)))
        DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_HOST_PTR, errcode_ret, mem)
    if (size == 0)
        DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_BUFFER_SIZE, errcode_ret, mem)

    if (dfMemAlloc(&dptr, &dieConfig, size) != DF_SUCCESS) {
        DFCL_SET_FUNCTION_VALUE_RETURN(CL_OUT_OF_HOST_MEMORY, errcode_ret, mem)
    }

    auto oclMem = DF_NEW(OpenclMemory(dptr));
    if (!oclMem) {
        dfMemFree(dptr);
        DFCL_SET_FUNCTION_VALUE_RETURN(CL_OUT_OF_HOST_MEMORY, errcode_ret, mem)
    }

    DFCL_SET_FUNCTION_VALUE_RETURN(CL_SUCCESS, errcode_ret, as_cl(oclMem)) 
}

// queue 可以用户异步拷贝, todo: 异步拷贝的实现
// event_wait_list 可以等待多个事件完成后再执行拷贝操作, todo
extern "C" CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteBuffer(cl_command_queue command_queue,
                            cl_mem buffer,
                            [[maybe_unused]] cl_bool blocking_write,
                            size_t offset,
                            size_t size,
                            const void* ptr,
                            [[maybe_unused]] cl_uint num_events_in_wait_list,
                            [[maybe_unused]] const cl_event* event_wait_list,
                            [[maybe_unused]] cl_event* event) {
    if (!is_valid(command_queue)) {
        return CL_INVALID_COMMAND_QUEUE;
    }
    if (!is_valid(buffer)) {
        return CL_INVALID_MEM_OBJECT;
    }
    if (!is_valid(ptr)) {
        return CL_INVALID_VALUE;
    }

    // todo: Define dies to use. pe dies.
    DFDieCoord upperLeft = {0, 0};
    DFDieCoord bottomRight = {0, 0};  // Example coordinates for two dies
    DFDieGrid dieGrid = {upperLeft, bottomRight};
    DFDieConfig dieConfig;
    dieConfig.type = DF_DIE_CONFIG_TYPE_GRID;
    dieConfig.grid = dieGrid;

    // auto queue = as_internal(command_queue);
    auto mem = as_internal(buffer);
    if (dfMemcpyHtoD(mem->get(), offset, &dieConfig, (void* )ptr, size, (DFMemcpyFlag)0) != DF_SUCCESS) {
        return CL_OUT_OF_HOST_MEMORY;
    }

    return CL_SUCCESS;
}

extern "C" CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadBuffer(cl_command_queue command_queue,
                            cl_mem buffer,
                            [[maybe_unused]] cl_bool blocking_write,
                            size_t offset,
                            size_t size,
                            void* ptr,
                            [[maybe_unused]] cl_uint num_events_in_wait_list,
                            [[maybe_unused]] const cl_event* event_wait_list,
                            [[maybe_unused]] cl_event* event) {
    if (!is_valid(command_queue)) {
        return CL_INVALID_COMMAND_QUEUE;
    }
    if (!is_valid(buffer)) {
        return CL_INVALID_MEM_OBJECT;
    }
    if (!is_valid(ptr)) {
        return CL_INVALID_VALUE;
    }

    // todo: Define dies to use. pe dies.
    DFDieCoord upperLeft = {0, 0};
    DFDieCoord bottomRight = {0, 0};  // Example coordinates for two dies
    DFDieGrid dieGrid = {upperLeft, bottomRight};
    DFDieConfig dieConfig;
    dieConfig.type = DF_DIE_CONFIG_TYPE_GRID;
    dieConfig.grid = dieGrid;

    // auto queue = as_internal(command_queue);
    auto mem = as_internal(buffer);
    if (dfMemcpyDtoH(ptr, mem->get(), offset, &dieConfig, size) != DF_SUCCESS) {
        return CL_OUT_OF_HOST_MEMORY;
    }

    return CL_SUCCESS;
}

extern "C" CL_API_ENTRY cl_int CL_API_CALL
clReleaseMemObject(cl_mem memobj) {
    if (!is_valid(memobj)) {
        return CL_INVALID_MEM_OBJECT;
    }
    auto oclMem = as_internal(memobj);
    oclMem->release();
    return CL_SUCCESS;
}