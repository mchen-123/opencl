#include "cl_program.hpp"
#include "cl_device.hpp"

extern "C" CL_API_ENTRY cl_program CL_API_CALL
clCreateProgramWithBinary(cl_context context, 
                                     cl_uint num_devices, 
                                     const cl_device_id* device_list,
                                     const size_t* lengths,
                                     const unsigned char** binaries,
                                     cl_int* binary_status,
                                     cl_int* errcode_ret) {
    cl_int result = CL_SUCCESS;
                                
    if (!is_valid(context)) {
        DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_CONTEXT, errcode_ret, nullptr)
    }
    if (num_devices == 0 || device_list == nullptr || binaries == nullptr || lengths == nullptr)
        DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_VALUE, errcode_ret, nullptr)
    
    auto ctx = as_internal(context);
    // todo: mulitple devices
    auto program = DF_NEW(OpenclProgram(ctx));
    for (cl_uint i = 0; i < num_devices; ++i) {
        auto dev = as_internal(device_list[i]);
        if (!is_valid(dev) || !ctx->getContexts()[dev->id()]) {
            if (binary_status) binary_status[i] = CL_INVALID_DEVICE;
            continue;
        }
        if (binaries[i] == nullptr || lengths[i] == 0) {
            if (binary_status != nullptr) binary_status[i] = CL_INVALID_VALUE;
            continue;            
        }
        DFModule module = nullptr;
        // todo：use dfModuleLoadData instread of dfModuleLoad
        if (dfModuleLoad(&module, KERNEL_EFL_FILE) != DF_SUCCESS) {
            if (binary_status != nullptr) binary_status[i] = CL_INVALID_BINARY;
            DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_BINARY, errcode_ret, nullptr)
        }
        program->setModule(module);
        if (binary_status != nullptr) {
            binary_status[i] = CL_SUCCESS;
        }
    }

    DFCL_SET_FUNCTION_VALUE_RETURN(CL_SUCCESS, errcode_ret, as_cl(program))
}

extern "C" CL_API_ENTRY cl_int CL_API_CALL
clReleaseProgram(cl_program  program) {
    if (!is_valid(program)) {
        return CL_INVALID_PROGRAM;
    }
    auto oclProgram = as_internal(program);
    oclProgram->release();
    return CL_SUCCESS;
}

extern "C" CL_API_ENTRY cl_int CL_API_CALL
clBuildProgram([[maybe_unused]] cl_program program, 
                      [[maybe_unused]] cl_uint num_devices, 
                      [[maybe_unused]] const cl_device_id* device_list,
                      [[maybe_unused]] const char* options,
                      [[maybe_unused]] void(CL_CALLBACK* pfn_notify)(cl_program program, void* user_data),
                      [[maybe_unused]] void* user_data) {
    // todo: check ctx, device, program
    return CL_SUCCESS;
}

extern "C" CL_API_ENTRY cl_kernel CL_API_CALL
clCreateKernel(cl_program program, const char* kernel_name, cl_int* errcode_ret) {
    cl_int result = CL_SUCCESS;

    if(!is_valid(program)) {
        DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_PROGRAM, errcode_ret, nullptr)
    }
    
    if (kernel_name == nullptr) {
        DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_VALUE, errcode_ret, nullptr)
    }

    auto prog = as_internal(program);

    DFFunction kernelFunc;
    if (dfModuleGetFunction(&kernelFunc, prog->getModule(), kernel_name)!= DF_SUCCESS) {
        DFCL_SET_FUNCTION_VALUE_RETURN(CL_INVALID_KERNEL_NAME, errcode_ret, nullptr)
    };

    auto oclKernel = DF_NEW(OpenclKernel(prog, kernel_name, kernelFunc));
    if (oclKernel == nullptr) {
        DFCL_SET_FUNCTION_VALUE_RETURN(CL_OUT_OF_HOST_MEMORY, errcode_ret, nullptr)
    }
    DFCL_SET_FUNCTION_VALUE_RETURN(CL_SUCCESS, errcode_ret, as_cl(oclKernel))
}

extern "C" CL_API_ENTRY cl_int CL_API_CALL
clReleaseKernel(cl_kernel kernel) {
    if (!is_valid(kernel)) {
        return CL_INVALID_KERNEL;
    }
    as_internal(kernel)->release();
    return CL_SUCCESS;
}

extern "C" CL_API_ENTRY cl_int CL_API_CALL
clSetKernelArg(cl_kernel kernel, [[maybe_unused]] cl_uint arg_index, size_t arg_size, const void* arg_value) {
    if (!is_valid(kernel)) {
        return CL_INVALID_KERNEL;
    }
    // todo: The actual validity check. whether arg_index exceeds the number of arguments for this kernel. 

    auto* oclKernel = as_internal(kernel);
    if (!oclKernel || oclKernel->type() != OpenclObject::OBJECT_TYPE_KERNEL) {
        return CL_INVALID_KERNEL;
    }

    auto* params = oclKernel->parameters();
    if (!params) {
        return CL_OUT_OF_HOST_MEMORY;
    }
    if (arg_size == sizeof(cl_mem) && arg_value) {
        cl_mem mem = *static_cast<const cl_mem*>(arg_value);
        auto memObj = as_internal(mem);
        if (memObj && OpenclObject::OBJECT_TYPE_MEM_OBJ == memObj->type() ) {
            params->add_mem(memObj);
            return CL_SUCCESS;
        }
        return CL_INVALID_MEM_OBJECT;
    } else {
        params->add(arg_value);
        return CL_SUCCESS;
    }
}