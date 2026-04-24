
#include <iostream>
#include <vector>
#include <string>
#include <rte_memory.h>
#include <rte_malloc.h>
#include <rte_launch.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_debug.h>
#include <gtest/gtest.h>
#include "dpdk_test_common.h"

// 定义测试内存大小
#define TEST_SIZE 1024
#define TEST_ALIGN 64

// 全局变量
void* test_ptr = nullptr;

// Mem测试夹具
class DpdkMemTest : public ::testing::Test {
protected:
    void* test_ptr = nullptr;

    void SetUp() override {
        test_ptr = nullptr;
    }

    void TearDown() override {
        if (test_ptr) {
            rte_free(test_ptr);
            test_ptr = nullptr;
        }
    }
};

// 测试对齐分配
TEST_F(DpdkMemTest, AlignedMalloc) {
    // 分配对齐内存
    test_ptr = rte_malloc("test_aligned", TEST_SIZE, TEST_ALIGN);
    ASSERT_NE(test_ptr, nullptr);

    // 验证对齐
    uintptr_t addr = (uintptr_t)test_ptr;
    ASSERT_EQ(addr % TEST_ALIGN, 0);

    // 释放内存
    rte_free(test_ptr);
    test_ptr = nullptr;
}

// 测试zmalloc（零初始化）
TEST_F(DpdkMemTest, Zmalloc) {
    // 分配并清零内存
    test_ptr = rte_zmalloc("test_zmem", TEST_SIZE, 0);
    ASSERT_NE(test_ptr, nullptr);

    // 验证内存已清零
    unsigned char* p = (unsigned char*)test_ptr;
    for (int i = 0; i < TEST_SIZE; i++) {
        ASSERT_EQ(p[i], 0);
    }

    // 释放内存
    rte_free(test_ptr);
    test_ptr = nullptr;
}

// 测试realloc
TEST_F(DpdkMemTest, Realloc) {
    // 初始分配
    test_ptr = rte_malloc("test_realloc", TEST_SIZE, 0);
    ASSERT_NE(test_ptr, nullptr);

    // 写入数据
    memset(test_ptr, 0xBB, TEST_SIZE);

    // 扩大内存
    void* new_ptr = rte_realloc(test_ptr, TEST_SIZE * 2, 0);
    ASSERT_NE(new_ptr, nullptr);
    test_ptr = new_ptr;

    // 验证原数据
    unsigned char* p = (unsigned char*)test_ptr;
    for (int i = 0; i < TEST_SIZE; i++) {
        ASSERT_EQ(p[i], 0xBB);
    }

    // 释放内存
    rte_free(test_ptr);
    test_ptr = nullptr;
}

// 测试socket特定分配
TEST_F(DpdkMemTest, SocketMalloc) {
    // 获取当前socket ID
    int socket_id = rte_socket_id();

    // 在指定socket上分配内存
    test_ptr = rte_malloc_socket("test_socket", TEST_SIZE, 0, socket_id);
    ASSERT_NE(test_ptr, nullptr);

    // 验证内存可用
    ASSERT_NE(test_ptr, nullptr);

    // 释放内存
    rte_free(test_ptr);
    test_ptr = nullptr;
}