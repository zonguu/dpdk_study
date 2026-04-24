#include <iostream>
#include <vector>
#include <string>
#include <rte_lcore.h>
#include <rte_launch.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_debug.h>
#include <gtest/gtest.h>
#include "dpdk_test_common.h"

// 定义per-lcore变量
RTE_DEFINE_PER_LCORE(int, per_lcore_var);

// Lcore测试夹具
class DpdkLcoreTest : public ::testing::Test {
protected:
    static int worker_result;
    unsigned int main_lcore_id = 0;
    unsigned int worker_lcore_id = 0;
    
    static int lcore_worker(void *arg) {
        int *value = (int*)arg;
        *value = 42;
        return 0;
    }

    void SetUp() override {
        main_lcore_id = rte_get_main_lcore();
    }

    void TearDown() override {}
};

int DpdkLcoreTest::worker_result = 0;

TEST_F(DpdkLcoreTest, BasicInfo) {
    EXPECT_GE(main_lcore_id, 0);
    
    unsigned int current_lcore = rte_lcore_id();
    EXPECT_EQ(current_lcore, main_lcore_id);
    
    unsigned int lcore_count = rte_lcore_count();
    EXPECT_GE(lcore_count, 1);
    
    std::cout << "Lcore information:" << std::endl;
    std::cout << "  - Main lcore: " << main_lcore_id << std::endl;
    std::cout << "  - Current lcore: " << current_lcore << std::endl;
    std::cout << "  - Total lcores: " << lcore_count << std::endl;
}

TEST_F(DpdkLcoreTest, Iteration) {
    unsigned int lcore_count = 0;
    unsigned int enabled_lcores = 0;
    unsigned int lcore_id;

    RTE_LCORE_FOREACH_WORKER(lcore_id) {
        lcore_count++;
        if (rte_lcore_is_enabled(lcore_id)) {
            enabled_lcores++;
            std::cout << "  - Lcore " << lcore_id << " is enabled" << std::endl;
        }
    }

    std::cout << "Lcore iteration:" << std::endl;
    std::cout << "  - Total lcores: " << lcore_count << std::endl;
    std::cout << "  - Enabled lcores: " << enabled_lcores << std::endl;
}

TEST_F(DpdkLcoreTest, TaskExecution) {
    unsigned int lcore_id;

    worker_lcore_id = 0;
    RTE_LCORE_FOREACH_WORKER(lcore_id) {
        if (rte_lcore_is_enabled(lcore_id)) {
            worker_lcore_id = lcore_id;
            break;
        }
    }

    if (worker_lcore_id == 0) {
        std::cout << "No worker lcore available, skipping task execution test" << std::endl;
        return;
    }

    worker_result = 0;
    int ret = rte_eal_remote_launch(lcore_worker, &worker_result, worker_lcore_id);
    ASSERT_EQ(ret, 0);

    rte_eal_wait_lcore(worker_lcore_id);
    EXPECT_EQ(worker_result, 42);
}

TEST_F(DpdkLcoreTest, PerLcoreVariable) {
    RTE_PER_LCORE(per_lcore_var) = 100;
    EXPECT_EQ(RTE_PER_LCORE(per_lcore_var), 100);
    std::cout << "Per-lcore variable on lcore " << rte_lcore_id()
              << ": " << RTE_PER_LCORE(per_lcore_var) << std::endl;
}
