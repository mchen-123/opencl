cmake_minimum_required(VERSION 3.0)

project(dfcl C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")

# ============ 先构建 runtime_mock ============
add_subdirectory(runtime_mock)

add_subdirectory(dfruntime)

add_compile_options(-fPIC)
add_compile_options(-Wall -Wextra -Wno-error)

add_library(thrive_opencl SHARED
    cl_icd.c
    cl_ext.c
    cl_platform.c
    cl_device.c
    cl_context.c
    cl_command.c
    cl_util.c

    # cl_memory.c
    # cl_program.c
    # cl_excute.c
)

# set_target_properties(thrive_opencl PROPERTIES
# LINK_FLAGS "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/icd_driver_export.map"
# )
target_include_directories(thrive_opencl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/install/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/dfruntime/
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime_mock/include
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime_mock/src
)

target_link_libraries(thrive_opencl PRIVATE dfcl-device runtime_mock ${CMAKE_CURRENT_SOURCE_DIR}/install/lib/libOpenCL.so pthread)

target_compile_definitions(thrive_opencl PRIVATE CL_TARGET_OPENCL_VERSION=220)
