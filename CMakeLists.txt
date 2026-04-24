cmake_minimum_required(VERSION 3.10)
project(dpdk_ring_study)

# 设置C++标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_TESTS "Enable tests" ON)
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

# 查找必要的包
find_package(PkgConfig REQUIRED)

# 查找DPDK
pkg_check_modules(DPDK REQUIRED libdpdk)

# 包含目录
include_directories(
    ${DPDK_INCLUDE_DIRS}
)

# 链接目录
link_directories(
    ${DPDK_LIBRARY_DIRS}
)

# 设置通用编译选项
set(COMMON_FLAGS "-Wall -Wextra -Wno-unused-parameter -Wno-unused-but-set-variable")

# 如果启用ASAN，添加ASAN标志
if(ENABLE_ASAN)
    MESSAGE(STATUS "AddressSanitizer enabled")
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

# 如果使用DPDK，需要设置特定的编译和链接选项
if(DPDK_FOUND)
    # 添加DPDK特定的编译选项
    set(COMMON_FLAGS "${COMMON_FLAGS} -include rte_config.h")
endif()

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} -mssse3")

# 如果启用测试，添加测试目标和GTest
if(ENABLE_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)

    # 包含GTest头文件目录
    include_directories(${GTEST_INCLUDE_DIRS})

    aux_source_directory(test/ TEST_SRCS)

    # 添加ring测试可执行文件
    add_executable(dpdk_test ${TEST_SRCS})
    target_link_libraries(dpdk_test
        ${DPDK_LIBRARIES}
        ${GTEST_LIBRARIES}
        pthread
        rt
        m
    )
    set_target_properties(dpdk_test PROPERTIES
        LINK_FLAGS "-Wl,--no-as-needed"
    )

    set_target_properties(dpdk_test PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output
    )

endif()