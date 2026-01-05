cmake_minimum_required(VERSION 3.0)
project(devices LANGUAGES CXX)

# ============ 全局配置 ============
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

add_library(dfcl-device SHARED
    devices.cpp
    dynlib.cpp
    dfruntime/dfcl-runtime.cpp
)

set_target_properties(dfcl-device PROPERTIES
    PREFIX "lib" # 确保有 lib 前缀
    OUTPUT_NAME "dfcl-device" # 输出文件名 dfcl-device → libdfcl-device.so
)

target_include_directories(dfcl-device PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../install
)

target_link_libraries(dfcl-device PRIVATE
    /home/mchen/opencl/damocl/install/lib/libOpenCL.so
    runtime_mock
)

target_link_options(dfcl-device PRIVATE -Wl,--export-dynamic)

target_compile_definitions(dfcl-device PRIVATE CL_TARGET_OPENCL_VERSION=220)
