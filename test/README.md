
# DPDK 测试套件

这个项目包含DPDK几个重要模块的单元测试，使用Google Test框架编写。

## 测试模块

1. **ring_test** - 环形队列测试
   - 基本功能测试
   - 批量操作测试
   - 统计功能测试
   - 容量测试
   - 边界条件测试

2. **mem_test** - 内存管理测试
   - 基本分配和释放测试
   - 对齐分配测试
   - 零初始化分配测试
   - 重新分配测试
   - Socket特定分配测试

3. **mempool_test** - 内存池测试
   - 创建和销毁测试
   - 分配和释放测试
   - 批量操作测试
   - 内存耗尽测试
   - 统计信息测试

4. **hash_test** - 哈希表测试
   - 创建和销毁测试
   - 基本操作测试
   - 批量操作测试
   - 统计信息测试
   - 冲突处理测试

5. **lcore_test** - 逻辑核心测试
   - 基本信息测试
   - 核心遍历测试
   - 任务执行测试
   - Per-lcore变量测试

6. **mbuf_test** - 消息缓冲区测试
   - 分配和释放测试
   - 数据操作测试
   - 链式MBUF测试
   - MBUF克隆测试
   - 数据长度调整测试

## 编译和运行

### 前置条件

- 安装DPDK开发库
- 安装Google Test框架
- CMake 3.10或更高版本

### 编译

```bash
mkdir build
cd build
cmake ..
make
```

### 运行所有测试

```bash
make test
```

### 运行单个测试

```bash
# 运行ring测试
./ring_test

# 运行mem测试
./mem_test

# 运行mempool测试
./mempool_test

# 运行hash测试
./hash_test

# 运行lcore测试
./lcore_test

# 运行mbuf测试
./mbuf_test
```

## 注意事项

1. 所有测试都使用`--no-huge`和`--no-pci`选项，因此不需要配置hugepages或PCI设备
2. 测试运行时需要足够的内存（默认配置为512MB）
3. 某些测试（如lcore_test）可能需要多个CPU核心才能完整执行
4. 每个测试使用不同的`--file-prefix`，可以同时运行多个测试

## 测试结果

所有测试都应该成功通过。如果某个测试失败，请检查：
- DPDK是否正确安装
- 系统是否有足够的内存
- 是否有足够的CPU核心
