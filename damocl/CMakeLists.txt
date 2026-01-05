cmake_minimum_required(VERSION 3.0)

project(damocl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# ============ 先构建 runtime_mock ============
add_subdirectory(runtime_mock)

add_subdirectory(devices)

add_compile_options(-fPIC)
add_compile_options(-Wall -Wextra -Werror)

add_library(damocl SHARED
    cl_icd.cpp
    cl_ext.cpp
    cl_platform.cpp
    cl_device.cpp

    # cl_context.cpp
    # cl_command.cpp
    # cl_memory.cpp
    # cl_program.cpp
    # cl_excute.cpp
)

# set_target_properties(damocl PROPERTIES
# LINK_FLAGS "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/icd_driver_export.map"
# )
target_include_directories(damocl PUBLIC
    /home/mchen/opencl/damocl/install/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/devices/include
    ${CMAKE_CURRENT_SOURCE_DIR}/devices/
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime_mock/include
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime_mock/src
)

target_link_libraries(damocl PRIVATE dfcl-device runtime_mock /home/mchen/opencl/damocl/install/lib/libOpenCL.so)

target_compile_definitions(damocl PRIVATE CL_TARGET_OPENCL_VERSION=220)
