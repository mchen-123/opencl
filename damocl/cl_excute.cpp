#include <CL/cl.h>

#include "cl_object.hpp"
#include "cl_program.hpp"
#include "cl_command.hpp"

extern "C" CL_API_ENTRY cl_int CL_API_CALL
clEnqueueNDRangeKernel(cl_command_queue command_queue, cl_kernel kernel, [[maybe_unused]] cl_uint work_dim,
               [[maybe_unused]] const size_t* global_work_offset, [[maybe_unused]] const size_t* global_work_size,
               [[maybe_unused]] const size_t* local_work_size, [[maybe_unused]] cl_uint num_events_in_wait_list,
               [[maybe_unused]] const cl_event* event_wait_list, [[maybe_unused]] cl_event* event) {
    if (!is_valid(command_queue)) {
        return CL_INVALID_COMMAND_QUEUE;
    }
    if (!is_valid(kernel)) {
        return CL_INVALID_KERNEL;
    }

    // todo: Define dies to use. pe dies, hardcode
    DFDieCoord upperLeft = {0, 0};
    DFDieCoord bottomRight = {0, 0};  // Example coordinates for two dies
    DFDieGrid dieGrid = {upperLeft, bottomRight};
    DFDieConfig dieConfig;
    dieConfig.type = DF_DIE_CONFIG_TYPE_GRID;
    dieConfig.grid = dieGrid;
    int peNum = 1;
    int rvNum = 1;
    uint64_t sharedMemSize = 0;

    auto oclKernel = as_runtime(kernel);
    auto oclCommandQueue = as_runtime(command_queue);

    std::vector<void *> kernelParamsArray;
    auto args = oclKernel->parameters();
    kernelParamsArray.reserve(args->size());
    for (size_t i = 0; i < args->size(); ++i) {
        auto arg = (*args)[i];
        if (arg.kind == kernelParameters::Arg::Kind::MEM) {
            auto tmp = static_cast<OpenclMemory*>(arg.value);
            kernelParamsArray[i] = tmp->getPointer();
        } else if (arg.kind == kernelParameters::Arg::Kind::POD) {
            kernelParamsArray[i] = arg.value;
        } else if (arg.kind == kernelParameters::Arg::Kind::LOCAL) {
            kernelParamsArray[i] = nullptr;
        }
    }

    DFResult err = dfLaunchKernel(oclKernel->getFunction(), &dieConfig, peNum, rvNum, sharedMemSize, oclCommandQueue->stream_, kernelParamsArray.data() ,nullptr);
    if (err != DF_SUCCESS) {
        return CL_INVALID_OPERATION;
    }
    return CL_SUCCESS;
}