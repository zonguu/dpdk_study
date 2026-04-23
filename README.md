# DPDK 无锁队列 (rte_ring) 示例

这个项目展示了如何使用 DPDK 的 rte_ring 实现无锁队列的基本操作，包括入队(enqueue)和出队(dequeue)操作。

## 项目概述

本项目包含一个使用 gtest 测试框架的 DPDK rte_ring 示例，展示了：
- 基本的入队和出队操作
- 批量操作 (burst operations)
- 环形队列统计功能
- 环形队列容量查询
- 边界条件测试

## 项目结构

```
dpdk_study/
├── CMakeLists.txt    # CMake构建配置
├── build.sh          # 构建脚本
├── README.md         # 项目说明文档
└── test/
    └── ring_test.cpp # rte_ring测试用例
```

## 依赖项

- DPDK (Data Plane Development Kit)
- gtest (Google Test Framework)
- CMake 3.10 或更高版本

## 安装依赖项

在 Ubuntu/Debian 系统上：
```bash
sudo apt-get install cmake libgtest-dev dpdk libdpdk-dev
```

## 构建和测试

本项目提供了一个便捷的构建脚本 `build.sh`，支持多种构建选项：

### 使用构建脚本

基本构建（包含测试）：
```bash
chmod +x build.sh
./build.sh
```

启用 AddressSanitizer（ASAN）进行内存检查：
```bash
./build.sh --asan
```

清理构建目录后重新构建：
```bash
./build.sh --clean
```

禁用测试：
```bash
./build.sh --no-tests
```

组合使用多个选项：
```bash
./build.sh --clean --asan
```

查看帮助信息：
```bash
./build.sh --help
```

### 手动构建

如果需要手动构建，可以执行以下步骤：

1. 创建构建目录：
```bash
mkdir build
cd build
```

2. 配置 CMake（启用 ASAN 和测试）：
```bash
cmake -DENABLE_ASAN=ON -DENABLE_TESTS=ON ..
```

3. 构建项目：
```bash
make -j$(nproc)
```

4. 运行测试：
```bash
ctest --output-on-failure
```

或者直接运行测试可执行文件：
```bash
./test/ring_test
```

## 测试说明

测试文件位于 `test/ring_test.cpp`，包含以下测试用例：

- **BasicFunctionality**：测试基本入队和出队操作
- **BurstOperations**：测试批量操作
- **Statistics**：测试环形队列统计功能
- **Capacity**：测试环形队列容量查询
- **BoundaryConditions**：测试边界条件，包括空队列出队和满队列入队

## 注意事项

1. DPDK 程序通常需要 root 权限或特定的用户权限
2. 在生产环境中使用前，请确保理解 DPDK 的内存管理机制
3. 启用 ASAN 会增加内存使用和运行时间，但有助于发现内存问题
4. 测试使用虚拟设备，不需要绑定真实的网卡

## 参考资料

- [DPDK 官方文档](https://doc.dpdk.org/)
- [rte_ring API 参考](https://doc.dpdk.org/api/rte__ring_8h.html)
- [AddressSanitizer 文档](https://github.com/google/sanitizers/wiki/AddressSanitizer)
