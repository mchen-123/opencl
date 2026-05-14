cmake_minimum_required(VERSION 3.5)

project(dfcl C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")

add_compile_options(-fPIC)
add_compile_options(-Wall -Wextra -Wno-error)

add_library(thrive_opencl SHARED
    # platform
    platform/clGetPlatformIDs.c
    platform/clGetPlatformInfo.c
    platform/clIcdGetPlatformIDsKHR.c

    # icd
    icd/cliIcdDispatchTableCreate.c
    icd/clGetExtensionFunctionAddress.c

    # device
    device/clGetDeviceIDs.c
    device/clGetDeviceInfo.c
    device/clRetainDevice.c
    device/clReleaseDevice.c
    device/clCreateSubDevices.c

    # context
    context/clCreateContext.c
    context/clRetainContext.c
    context/clReleaseContext.c

    # commandqueue
    commandqueue/clCreateCommandQueue.c
    commandqueue/clReleaseCommandQueue.c

    # dfruntime
    dfruntime/dfruntime.c
    dfruntime/dynlib.c
)

# set_target_properties(thrive_opencl PROPERTIES
# LINK_FLAGS "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/icd_driver_export.map"
# )
target_include_directories(thrive_opencl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/../inc/
    ${CMAKE_CURRENT_SOURCE_DIR}/install/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/dfruntime/
    ${CMAKE_CURRENT_SOURCE_DIR}/../runtime_mock/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../runtime_mock/src
)

target_link_libraries(thrive_opencl PRIVATE runtime_mock ${CMAKE_CURRENT_SOURCE_DIR}/install/lib/libOpenCL.so pthread dl)

target_compile_definitions(thrive_opencl PRIVATE CL_TARGET_OPENCL_VERSION=300)
