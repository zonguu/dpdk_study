#include <iostream>
#include <vector>
#include <string>
#include <rte_ring.h>
#include <rte_mbuf.h>
#include <rte_malloc.h>
#include <rte_memory.h>
#include <rte_launch.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_debug.h>
#include <gtest/gtest.h>
#include "dpdk_test_common.h"

// 定义环形队列大小
#define RING_SIZE 4096
// 定义测试数据数量
#define TEST_COUNT 1000

// 全局变量
class DpdkRingTest : public ::testing::Test {
protected:
    rte_ring* test_ring = nullptr;

    void SetUp() override {
        static int ring_counter = 0;
        char ring_name[64];
        snprintf(ring_name, sizeof(ring_name), "test_ring_%d", ring_counter++);

        test_ring = rte_ring_create(ring_name, RING_SIZE, rte_socket_id(), RING_F_SC_DEQ);
        ASSERT_NE(test_ring, nullptr) << "Failed to create ring";
    }

    void TearDown() override {
        if (test_ring) {
            rte_ring_free(test_ring);
            test_ring = nullptr;
        }
    }
};

// 测试rte_ring基本功能
TEST_F(DpdkRingTest, BasicFunctionality) {
    // 环形队列已在SetUp中创建

    // 创建测试数据
    int* data[TEST_COUNT];
    for (int i = 0; i < TEST_COUNT; i++) {
        data[i] = new int(i);
    }

    // 入队，rte_ring_enqueue_bulk返回实际入队的元素数量
    unsigned int enqueued = rte_ring_enqueue_bulk(test_ring, (void**)data, TEST_COUNT, nullptr);
    ASSERT_EQ(enqueued, TEST_COUNT);

    // 出队，rte_ring_dequeue_bulk返回实际出队的元素数量
    int* dequeued_data[TEST_COUNT];
    unsigned int dequeued = rte_ring_dequeue_bulk(test_ring, (void**)dequeued_data, TEST_COUNT, nullptr);
    ASSERT_EQ(dequeued, TEST_COUNT);

    // 验证数据
    for (int i = 0; i < TEST_COUNT; i++) {
        ASSERT_EQ(*dequeued_data[i], i);
        delete dequeued_data[i];
    }

    // 清理环形队列
    rte_ring_free(test_ring);
    test_ring = nullptr;
}

// 测试rte_ring批量操作
TEST_F(DpdkRingTest, BurstOperations) {
    // 环形队列已在SetUp中创建

    // 批量入队
    int* data[TEST_COUNT / 2];
    for (int i = 0; i < TEST_COUNT / 2; i++) {
        data[i] = new int(i);
    }

    unsigned int enqueued = rte_ring_enqueue_burst(test_ring, (void**)data, TEST_COUNT / 2, nullptr);
    EXPECT_EQ(enqueued, TEST_COUNT / 2);

    // 批量出队
    int* dequeued_data[TEST_COUNT / 2];
    unsigned int dequeued = rte_ring_dequeue_burst(test_ring, (void**)dequeued_data, TEST_COUNT / 2, nullptr);
    EXPECT_EQ(dequeued, TEST_COUNT / 2);

    // 验证数据
    for (unsigned int i = 0; i < dequeued; i++) {
        ASSERT_EQ(*dequeued_data[i], i);
        delete dequeued_data[i];
    }

    // 清理环形队列
    rte_ring_free(test_ring);
    test_ring = nullptr;
}

// 测试rte_ring统计功能
TEST_F(DpdkRingTest, Statistics) {
    // 环形队列已在SetUp中创建

    // 获取初始统计信息
    std::cout << "Ring statistics before operations:" << std::endl;
    std::cout << "  - Count: " << rte_ring_count(test_ring) << std::endl;
    std::cout << "  - Free count: " << rte_ring_free_count(test_ring) << std::endl;

    // 创建测试数据
    int* data[TEST_COUNT];
    for (int i = 0; i < TEST_COUNT; i++) {
        data[i] = new int(i);
    }

    // 入队，rte_ring_enqueue_bulk返回实际入队的元素数量
    unsigned int enqueued = rte_ring_enqueue_bulk(test_ring, (void**)data, TEST_COUNT, nullptr);
    ASSERT_EQ(enqueued, TEST_COUNT);

    // 获取入队后的统计信息
    std::cout << "Ring statistics after enqueue:" << std::endl;
    std::cout << "  - Count: " << rte_ring_count(test_ring) << std::endl;
    std::cout << "  - Free count: " << rte_ring_free_count(test_ring) << std::endl;

    // 出队，rte_ring_dequeue_bulk返回实际出队的元素数量
    int* dequeued_data[TEST_COUNT];
    unsigned int dequeued = rte_ring_dequeue_bulk(test_ring, (void**)dequeued_data, TEST_COUNT, nullptr);
    ASSERT_EQ(dequeued, TEST_COUNT);

    // 释放内存
    for (int i = 0; i < TEST_COUNT; i++) {
        delete dequeued_data[i];
    }

    // 获取操作后的统计信息
    std::cout << "Ring statistics after dequeue:" << std::endl;
    std::cout << "  - Count: " << rte_ring_count(test_ring) << std::endl;
    std::cout << "  - Free count: " << rte_ring_free_count(test_ring) << std::endl;

    // 清理环形队列
    rte_ring_free(test_ring);
    test_ring = nullptr;
}

// 测试rte_ring容量
TEST_F(DpdkRingTest, Capacity) {
    // 环形队列已在SetUp中创建

    // 获取环形队列容量
    // 注意：rte_ring的实际容量是RING_SIZE-1，因为环形队列需要至少一个空闲位置来区分满队列和空队列
    unsigned int capacity = rte_ring_get_capacity(test_ring);
    EXPECT_EQ(capacity, RING_SIZE - 1);

    // 获取环形队列空闲空间
    unsigned int free_count = rte_ring_free_count(test_ring);
    EXPECT_EQ(free_count, capacity);

    // 获取环形队列元素数量
    unsigned int count = rte_ring_count(test_ring);
    EXPECT_EQ(count, 0);

    // 清理环形队列
    rte_ring_free(test_ring);
    test_ring = nullptr;
}

// 测试rte_ring边界条件
TEST_F(DpdkRingTest, BoundaryConditions) {
    // 环形队列已在SetUp中创建

    // 测试空队列出队
    void* obj;
    int ret = rte_ring_dequeue(test_ring, &obj);
    EXPECT_EQ(ret, -ENOENT);

    // 测试满队列入队
    // 注意：rte_ring的实际容量是RING_SIZE-1，因为环形队列需要至少一个空闲位置来区分满队列和空队列
    unsigned int actual_capacity = RING_SIZE - 1;
    int* data[RING_SIZE];
    for (unsigned int i = 0; i < actual_capacity; i++) {
        data[i] = new int(i);
    }

    // 入队，rte_ring_enqueue_bulk返回实际入队的元素数量
    unsigned int enqueued = rte_ring_enqueue_bulk(test_ring, (void**)data, actual_capacity, nullptr);
    ASSERT_EQ(enqueued, actual_capacity);

    // 尝试向满队列中添加元素
    int* extra = new int(actual_capacity);
    ret = rte_ring_enqueue(test_ring, extra);
    EXPECT_EQ(ret, -ENOBUFS);
    delete extra;

    // 清空队列，rte_ring_dequeue_bulk返回实际出队的元素数量
    int* dequeued_data[RING_SIZE];
    unsigned int dequeued = rte_ring_dequeue_bulk(test_ring, (void**)dequeued_data, actual_capacity, nullptr);
    ASSERT_EQ(dequeued, actual_capacity);

    // 释放内存
    for (unsigned int i = 0; i < actual_capacity; i++) {
        delete dequeued_data[i];
    }

    // 清理环形队列
    rte_ring_free(test_ring);
}