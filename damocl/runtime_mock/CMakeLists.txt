cmake_minimum_required(VERSION 3.10)
project(runtime_mock)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加动态库构建
add_library(runtime_mock SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/src/runtime_mock.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/context.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/error.cpp
)
target_include_directories(runtime_mock PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src)

# 设置安装路径
# install(TARGETS runtime_mock
# LIBRARY DESTINATION lib
# ARCHIVE DESTINATION lib)