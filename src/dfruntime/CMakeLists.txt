cmake_minimum_required(VERSION 3.5)
project(devices LANGUAGES C)

# ============ 全局配置 ============
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")

add_library(dfcl-device SHARED
    dfruntime.c
    dynlib.c
)

set_target_properties(dfcl-device PROPERTIES
    PREFIX "lib" # 确保有 lib 前缀
    OUTPUT_NAME "dfcl-device" # 输出文件名 dfcl-device → libdfcl-device.so
)

target_include_directories(dfcl-device PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../install/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../
)

target_link_libraries(dfcl-device PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../install/lib/libOpenCL.so
    runtime_mock
)

target_link_options(dfcl-device PRIVATE -Wl,--export-dynamic)

target_compile_definitions(dfcl-device PRIVATE CL_TARGET_OPENCL_VERSION=220 _GNU_SOURCE)
