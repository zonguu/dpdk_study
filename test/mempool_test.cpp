
#include <iostream>
#include <vector>
#include <string>
#include <rte_mempool.h>
#include <rte_malloc.h>
#include <rte_memory.h>
#include <rte_launch.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_debug.h>
#include <gtest/gtest.h>
#include "dpdk_test_common.h"

#define MEMPOOL_SIZE       256
#define MEMPOOL_CACHE_SIZE  32
#define MEMPOOL_ELT_SIZE    128

// Mempool测试夹具
class DpdkMempoolTest : public ::testing::Test {
protected:
    struct rte_mempool* test_mempool = nullptr;

    void SetUp() override {
        static int pool_counter = 0;
        char pool_name[64];
        snprintf(pool_name, sizeof(pool_name), "mempool_%d", pool_counter++);

        test_mempool = rte_mempool_create(
            pool_name,
            MEMPOOL_SIZE,
            MEMPOOL_ELT_SIZE,
            MEMPOOL_CACHE_SIZE,
            0,
            NULL, NULL,
            NULL, NULL,
            rte_socket_id(),
            0
        );

        ASSERT_NE(test_mempool, nullptr) << "Failed to create mempool";
    }

    void TearDown() override {
        if (test_mempool) {
            rte_mempool_free(test_mempool);
            test_mempool = nullptr;
        }
    }
};

// 测试内存池创建和销毁
TEST_F(DpdkMempoolTest, CreateAndDestroy) {
    // 内存池已在SetUp中创建

    // 验证内存池容量
    EXPECT_EQ(rte_mempool_avail_count(test_mempool), MEMPOOL_SIZE);

    // 销毁内存池
    rte_mempool_free(test_mempool);
    test_mempool = nullptr;
}

// 测试内存池分配和释放
TEST_F(DpdkMempoolTest, AllocAndFree) {
    // 内存池已在SetUp中创建

    // 分配元素
    void* obj = nullptr;
    int ret = rte_mempool_get(test_mempool, &obj);
    ASSERT_EQ(ret, 0);
    ASSERT_NE(obj, nullptr);

    // 验证可用数量减少
    EXPECT_EQ(rte_mempool_avail_count(test_mempool), MEMPOOL_SIZE - 1);

    // 释放元素
    rte_mempool_put(test_mempool, obj);

    // 验证可用数量恢复
    EXPECT_EQ(rte_mempool_avail_count(test_mempool), MEMPOOL_SIZE);

    // 销毁内存池
    rte_mempool_free(test_mempool);
    test_mempool = nullptr;
}

// 测试批量分配和释放
TEST_F(DpdkMempoolTest, BulkAllocAndFree) {
    // 内存池已在SetUp中创建

    // 批量分配
    const int bulk_count = 32;
    void* objs[bulk_count];
    int ret = rte_mempool_get_bulk(test_mempool, objs, bulk_count);
    ASSERT_EQ(ret, 0);

    // 验证可用数量减少
    EXPECT_EQ(rte_mempool_avail_count(test_mempool), MEMPOOL_SIZE - bulk_count);

    // 批量释放
    rte_mempool_put_bulk(test_mempool, objs, bulk_count);

    // 验证可用数量恢复
    EXPECT_EQ(rte_mempool_avail_count(test_mempool), MEMPOOL_SIZE);

    // 销毁内存池
    rte_mempool_free(test_mempool);
    test_mempool = nullptr;
}

// 测试内存池耗尽情况
TEST_F(DpdkMempoolTest, Exhaustion) {
    // 内存池已在SetUp中创建

    // 分配所有元素
    void* objs[MEMPOOL_SIZE];
    int ret = rte_mempool_get_bulk(test_mempool, objs, MEMPOOL_SIZE);
    ASSERT_EQ(ret, 0);

    // 验证内存池已空
    EXPECT_EQ(rte_mempool_avail_count(test_mempool), 0);

    // 尝试分配更多元素，应该失败
    void* extra_obj;
    ret = rte_mempool_get(test_mempool, &extra_obj);
    EXPECT_NE(ret, 0);

    // 释放所有元素
    rte_mempool_put_bulk(test_mempool, objs, MEMPOOL_SIZE);

    // 验证可用数量恢复
    EXPECT_EQ(rte_mempool_avail_count(test_mempool), MEMPOOL_SIZE);

    // 销毁内存池
    rte_mempool_free(test_mempool);
    test_mempool = nullptr;
}

// 测试内存池统计信息
TEST_F(DpdkMempoolTest, Statistics) {
    // 内存池已在SetUp中创建

    // 获取初始统计信息
    std::cout << "Mempool statistics:" << std::endl;
    std::cout << "  - Available: " << rte_mempool_avail_count(test_mempool) << std::endl;
    std::cout << "  - In use: " << rte_mempool_in_use_count(test_mempool) << std::endl;

    // 分配部分元素
    const int alloc_count = 64;
    void* objs[alloc_count];
    rte_mempool_get_bulk(test_mempool, objs, alloc_count);

    // 获取分配后的统计信息
    std::cout << "After allocation:" << std::endl;
    std::cout << "  - Available: " << rte_mempool_avail_count(test_mempool) << std::endl;
    std::cout << "  - In use: " << rte_mempool_in_use_count(test_mempool) << std::endl;

    // 验证统计信息
    EXPECT_EQ(rte_mempool_avail_count(test_mempool), MEMPOOL_SIZE - alloc_count);
    EXPECT_EQ(rte_mempool_in_use_count(test_mempool), alloc_count);

    // 释放元素
    rte_mempool_put_bulk(test_mempool, objs, alloc_count);

    // 销毁内存池
    rte_mempool_free(test_mempool);
    test_mempool = nullptr;
}