cmake_minimum_required(VERSION 3.5)

project(OpenCL_Wrapper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

add_subdirectory(runtime_mock)
add_subdirectory(src)
add_subdirectory(tests)
