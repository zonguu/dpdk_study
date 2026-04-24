#include <iostream>
#include <vector>
#include <string>
#include <rte_hash.h>
#include <rte_jhash.h>
#include <rte_malloc.h>
#include <rte_memory.h>
#include <rte_launch.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_debug.h>
#include <gtest/gtest.h>
#include "dpdk_test_common.h"

// 定义哈希表参数
#define HASH_ENTRIES 1024
#define HASH_KEY_LEN 8
#define HASH_NAME "test_hash"

// 哈希表测试夹具
class DpdkHashTest : public ::testing::Test {
protected:
    struct rte_hash* hash = nullptr;

    void SetUp() override {
        static int hash_counter = 0;
        char hash_name[64];
        snprintf(hash_name, sizeof(hash_name), "%s_%d", HASH_NAME, hash_counter++);
        
        struct rte_hash_parameters hash_params = {
            .name = hash_name,
            .entries = HASH_ENTRIES,
            .key_len = HASH_KEY_LEN,
            .hash_func = rte_jhash,
            .hash_func_init_val = 0,
            .socket_id = rte_socket_id(),
            .extra_flag = 0
        };

        hash = rte_hash_create(&hash_params);
        ASSERT_NE(hash, nullptr) << "Failed to create hash table";
    }

    void TearDown() override {
        if (hash) {
            rte_hash_free(hash);
            hash = nullptr;
        }
    }
};

// 测试用例保持不变，只是移除main函数
TEST_F(DpdkHashTest, CreateAndDestroy) {
    ASSERT_NE(hash, nullptr);
}

TEST_F(DpdkHashTest, BasicOperations) {
    uint64_t key = 100;
    int data = 1000;
    int ret = rte_hash_add_key_data(hash, &key, &data);
    ASSERT_EQ(ret, 0);

    void* return_data = nullptr;
    hash_sig_t sig = rte_hash_hash(hash, &key);
    ret = rte_hash_lookup_with_hash_data(hash, &key, sig, &return_data);
    ASSERT_GE(ret, 0);
    ASSERT_NE(return_data, nullptr);
    EXPECT_EQ(*(int*)return_data, 1000);

    ret = rte_hash_del_key_with_hash(hash, &key, sig);
    ASSERT_GE(ret, 0);

    return_data = nullptr;
    ret = rte_hash_lookup_with_hash_data(hash, &key, sig, &return_data);
    EXPECT_EQ(ret, -ENOENT);
}

TEST_F(DpdkHashTest, BulkOperations) {
    const int count = 100;
    uint64_t* keys = new uint64_t[count];
    int* data = new int[count];
    
    for (int i = 0; i < count; i++) {
        keys[i] = i;
        data[i] = i * 100;
        int ret = rte_hash_add_key_data(hash, &keys[i], &data[i]);
        ASSERT_EQ(ret, 0);
    }

    for (int i = 0; i < count; i++) {
        void* return_data = nullptr;
        hash_sig_t sig = rte_hash_hash(hash, &keys[i]);
        int ret = rte_hash_lookup_with_hash_data(hash, &keys[i], sig, &return_data);
        ASSERT_GE(ret, 0);
        ASSERT_NE(return_data, nullptr);
        EXPECT_EQ(*(int*)return_data, i * 100);
    }

    for (int i = 0; i < count; i++) {
        hash_sig_t sig = rte_hash_hash(hash, &keys[i]);
        int ret = rte_hash_del_key_with_hash(hash, &keys[i], sig);
        ASSERT_GE(ret, 0);
    }

    delete[] keys;
    delete[] data;
}

TEST_F(DpdkHashTest, Statistics) {
    const int count = 50;
    uint64_t* keys = new uint64_t[count];
    int* data = new int[count];
    
    for (int i = 0; i < count; i++) {
        keys[i] = i;
        data[i] = i * 10;
        rte_hash_add_key_data(hash, &keys[i], &data[i]);
    }

    for (int i = 0; i < count; i++) {
        void* return_data = nullptr;
        hash_sig_t sig = rte_hash_hash(hash, &keys[i]);
        int ret = rte_hash_lookup_with_hash_data(hash, &keys[i], sig, &return_data);
        EXPECT_GE(ret, 0);
        EXPECT_EQ(*(int*)return_data, i * 10);
    }

    for (int i = 0; i < count; i++) {
        hash_sig_t sig = rte_hash_hash(hash, &keys[i]);
        rte_hash_del_key_with_hash(hash, &keys[i], sig);
    }

    delete[] keys;
    delete[] data;
}

TEST_F(DpdkHashTest, CollisionHandling) {
    const int count = 200;
    uint64_t* keys = new uint64_t[count];
    int* data = new int[count];
    
    for (int i = 0; i < count; i++) {
        keys[i] = i;
        data[i] = i * 10;
        int ret = rte_hash_add_key_data(hash, &keys[i], &data[i]);
        ASSERT_EQ(ret, 0);
    }

    for (int i = 0; i < count; i++) {
        void* return_data = nullptr;
        hash_sig_t sig = rte_hash_hash(hash, &keys[i]);
        int ret = rte_hash_lookup_with_hash_data(hash, &keys[i], sig, &return_data);
        ASSERT_GE(ret, 0);
        ASSERT_NE(return_data, nullptr);
        EXPECT_EQ(*(int*)return_data, i * 10);
    }

    for (int i = 0; i < count; i++) {
        hash_sig_t sig = rte_hash_hash(hash, &keys[i]);
        rte_hash_del_key_with_hash(hash, &keys[i], sig);
    }

    delete[] keys;
    delete[] data;
}
