#include <gtest/gtest.h>
#include "dpdk_test_common.h"

int main(int argc, char **argv) {
    // 初始化DPDK EAL
    if (DpdkTestHelper::init_eal(argc, argv) < 0) {
        return -1;
    }

    // 初始化Google Test
    ::testing::InitGoogleTest(&argc, argv);

    // 运行所有测试
    return RUN_ALL_TESTS();
}
