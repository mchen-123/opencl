cmake_minimum_required(VERSION 3.0)
project(tests LANGUAGES CXX)

# ============ 全局配置 ============
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# 公共链接库
set(COMMON_LINK_LIBS thrive_opencl)

# ============ 测试目标列表 ============
set(TESTS
    test_platform
    test_device
    test_context
    # test_commandqueue
    # test_memory
    test_program
    # test_subdevice
)

# ============ 批量添加可执行文件 ============
foreach(test_name IN LISTS TESTS)
    add_executable(${test_name}
        ${test_name}.cpp
    )

    target_include_directories(${test_name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/install/include/CL/
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/runtime_mock/src
    )

    target_link_libraries(${test_name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/install/lib/libOpenCL.so
        ${COMMON_LINK_LIBS}
    )

    target_compile_definitions(${test_name} PRIVATE
        CL_TARGET_OPENCL_VERSION=220
    )
endforeach()