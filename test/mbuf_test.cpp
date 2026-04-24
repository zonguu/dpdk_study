#include <iostream>
#include <vector>
#include <string>
#include <rte_mbuf.h>
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

#define NUM_MBUFS       256
#define MBUF_CACHE_SIZE 32
#define MBUF_SIZE       2048
#define TEST_DATA_SIZE  128

// Mbuf测试夹具
class DpdkMbufTest : public ::testing::Test {
protected:
    struct rte_mempool* mbuf_pool;

    void SetUp() override {
        static int pool_counter = 0;
        char pool_name[64];
        snprintf(pool_name, sizeof(pool_name), "mbuf_pool_%d", pool_counter++);

        mbuf_pool = rte_pktmbuf_pool_create(
            pool_name,
            NUM_MBUFS,
            MBUF_CACHE_SIZE,
            0,
            MBUF_SIZE,
            rte_socket_id()
        );
        ASSERT_NE(mbuf_pool, nullptr) << "Failed to create mbuf pool";
    }

    void TearDown() override {
        if (mbuf_pool) {
            rte_mempool_free(mbuf_pool);
            mbuf_pool = nullptr;
        }
    }
};

// 测试MBUF分配和释放
TEST_F(DpdkMbufTest, AllocAndFree) {
    // 分配MBUF
    struct rte_mbuf *mbuf = rte_pktmbuf_alloc(mbuf_pool);
    ASSERT_NE(mbuf, nullptr);

    // 验证MBUF基本属性
    EXPECT_EQ(mbuf->pool, mbuf_pool);
    EXPECT_EQ(rte_pktmbuf_data_len(mbuf), 0);
    EXPECT_EQ(rte_pktmbuf_pkt_len(mbuf), 0);

    // 释放MBUF
    rte_pktmbuf_free(mbuf);
}

// 测试MBUF数据操作
TEST_F(DpdkMbufTest, DataOperations) {
    // 分配MBUF
    struct rte_mbuf *mbuf = rte_pktmbuf_alloc(mbuf_pool);
    ASSERT_NE(mbuf, nullptr);

    // 准备测试数据
    uint8_t test_data[TEST_DATA_SIZE];
    for (int i = 0; i < TEST_DATA_SIZE; i++) {
        test_data[i] = i % 256;
    }

    // 复制数据到MBUF
    char *data_ptr = rte_pktmbuf_append(mbuf, TEST_DATA_SIZE);
    ASSERT_NE(data_ptr, nullptr);
    rte_memcpy(data_ptr, test_data, TEST_DATA_SIZE);

    // 验证数据长度
    EXPECT_EQ(rte_pktmbuf_data_len(mbuf), TEST_DATA_SIZE);
    EXPECT_EQ(rte_pktmbuf_pkt_len(mbuf), TEST_DATA_SIZE);

    // 验证数据内容
    uint8_t *read_ptr = (uint8_t*)rte_pktmbuf_mtod(mbuf, char*);
    for (int i = 0; i < TEST_DATA_SIZE; i++) {
        EXPECT_EQ(read_ptr[i], test_data[i]);
    }

    // 释放MBUF
    rte_pktmbuf_free(mbuf);
}

// 测试MBUF链式操作
TEST_F(DpdkMbufTest, ChainedMbufs) {
    // 分配第一个MBUF
    struct rte_mbuf *mbuf1 = rte_pktmbuf_alloc(mbuf_pool);
    ASSERT_NE(mbuf1, nullptr);

    // 分配第二个MBUF
    struct rte_mbuf *mbuf2 = rte_pktmbuf_alloc(mbuf_pool);
    ASSERT_NE(mbuf2, nullptr);

    // 在第一个MBUF中添加数据
    char *data1 = rte_pktmbuf_append(mbuf1, 64);
    ASSERT_NE(data1, nullptr);
    rte_memcpy(data1, "First segment data", 18);

    // 在第二个MBUF中添加数据
    char *data2 = rte_pktmbuf_append(mbuf2, 64);
    ASSERT_NE(data2, nullptr);
    rte_memcpy(data2, "Second segment data", 19);

    // 链接两个MBUF
    rte_pktmbuf_chain(mbuf1, mbuf2);

    // 验证链式MBUF
    EXPECT_EQ(mbuf1->next, mbuf2);
    EXPECT_EQ(rte_pktmbuf_pkt_len(mbuf1), 128);
    EXPECT_EQ(rte_pktmbuf_data_len(mbuf1), 64);

    // 释放链式MBUF
    rte_pktmbuf_free(mbuf1);
}

// 测试MBUF克隆
TEST_F(DpdkMbufTest, Clone) {
    // 分配原始MBUF
    struct rte_mbuf *orig = rte_pktmbuf_alloc(mbuf_pool);
    ASSERT_NE(orig, nullptr);

    // 在原始MBUF中添加数据
    char *data = rte_pktmbuf_append(orig, 64);
    ASSERT_NE(data, nullptr);
    rte_memcpy(data, "Test data for cloning", 21);

    // 克隆MBUF
    struct rte_mbuf *clone = rte_pktmbuf_clone(orig, mbuf_pool);
    ASSERT_NE(clone, nullptr);

    // 验证克隆MBUF
    EXPECT_EQ(rte_pktmbuf_pkt_len(clone), rte_pktmbuf_pkt_len(orig));
    EXPECT_EQ(rte_pktmbuf_data_len(clone), rte_pktmbuf_data_len(orig));

    // 释放MBUF
    rte_pktmbuf_free(clone);
    rte_pktmbuf_free(orig);
}

// 测试MBUF调整数据长度
TEST_F(DpdkMbufTest, AdjustDataLength) {
    // 分配MBUF
    struct rte_mbuf *mbuf = rte_pktmbuf_alloc(mbuf_pool);
    ASSERT_NE(mbuf, nullptr);

    // 在MBUF中添加数据
    char *data = rte_pktmbuf_append(mbuf, 128);
    ASSERT_NE(data, nullptr);
    rte_memcpy(data, "Test data", 9);

    // 验证数据长度
    EXPECT_EQ(rte_pktmbuf_data_len(mbuf), 128);

    // 调整数据长度
    rte_pktmbuf_adj(mbuf, 64);

    // 验证调整后的数据长度
    EXPECT_EQ(rte_pktmbuf_data_len(mbuf), 64);

    // 释放MBUF
    rte_pktmbuf_free(mbuf);
}
