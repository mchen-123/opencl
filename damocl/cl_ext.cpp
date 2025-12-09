#include <CL/cl.h>
#include <CL/cl_ext.h>
#include <cl_khr_icd2.h>
#include <string_view>
#include <array>

CL_API_ENTRY void* CL_API_CALL
clGetExtensionFunctionAddress(const char* name) {
    using namespace std::literals;

    const auto extensions = std::array{
        std::pair{"clIcdGetPlatformIDsKHR"sv, reinterpret_cast<void*>(clIcdGetPlatformIDsKHR)},
        #ifdef CL_ENABLE_ICD2
            std::pair{"clIcdGetFunctionAddressForPlatformKHR"sv, reinterpret_cast<void*>(clIcdGetFunctionAddressForPlatformKHR)},
            std::pair{"clIcdSetPlatformDispatchDataKHR"sv, reinterpret_cast<void*>(clIcdSetPlatformDispatchDataKHR)}
        #endif // CL_ENABLE_ICD2
    };

    for (const auto& [ext_name, func] : extensions) {
        if (name == ext_name) {
            return func;
        }
    }
    return nullptr;

}