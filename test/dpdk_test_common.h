
#ifndef DPDK_TEST_COMMON_H
#define DPDK_TEST_COMMON_H

#include <rte_eal.h>
#include <string>
#include <vector>
#include <iostream>

/**
 * @brief DPDK测试通用辅助类
 * 
 * 这个类提供DPDK EAL初始化和参数解析的通用功能
 */
class DpdkTestHelper {
public:
    /**
     * @brief 初始化DPDK EAL
     * 
     * @param argc 命令行参数数量
     * @param argv 命令行参数数组
     * @return int 成功返回0，失败返回-1
     */
    static int init_eal(int argc, char** argv) {
        // 解析DPDK EAL参数
        std::vector<char*> eal_argv;

        // 添加程序名
        eal_argv.push_back(argv[0]);

        // 默认参数
        bool use_huge = false;
        bool use_pci = false;
        std::string file_prefix = "test";
        int memory = 512;
        int channels = 4;

        // 解析命令行参数
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];

            if (arg == "--use-huge") {
                use_huge = true;
            } else if (arg == "--use-pci") {
                use_pci = true;
            } else if (arg == "--file-prefix" && i + 1 < argc) {
                file_prefix = argv[++i];
            } else if (arg == "-m" && i + 1 < argc) {
                memory = std::atoi(argv[++i]);
            } else if (arg == "-n" && i + 1 < argc) {
                channels = std::atoi(argv[++i]);
            } else if (arg == "--help") {
                print_help(argv[0]);
                return -1;
            } else if (arg.find("--") == 0) {
                // 其他以--开头的参数传递给DPDK
                eal_argv.push_back(argv[i]);
            }
        }

        // 添加EAL参数
        if (!use_huge) {
            eal_argv.push_back(const_cast<char*>("--no-huge"));
        }
        if (!use_pci) {
            eal_argv.push_back(const_cast<char*>("--no-pci"));
        }

        // 添加文件前缀
        std::string prefix_arg = "--file-prefix=" + file_prefix;
        eal_argv.push_back(const_cast<char*>(prefix_arg.c_str()));

        // 添加内存大小
        std::string mem_arg = "-m";
        eal_argv.push_back(const_cast<char*>(mem_arg.c_str()));
        std::string mem_value = std::to_string(memory);
        eal_argv.push_back(const_cast<char*>(mem_value.c_str()));

        // 添加内存通道数
        std::string chan_arg = "-n";
        eal_argv.push_back(const_cast<char*>(chan_arg.c_str()));
        std::string chan_value = std::to_string(channels);
        eal_argv.push_back(const_cast<char*>(chan_value.c_str()));

        // 打印EAL参数
        std::cout << "DPDK EAL arguments:" << std::endl;
        for (size_t i = 0; i < eal_argv.size(); i++) {
            std::cout << "  [" << i << "] " << eal_argv[i] << std::endl;
        }

        // 初始化EAL
        int ret = rte_eal_init(eal_argv.size(), eal_argv.data());
        if (ret < 0) {
            std::cerr << "Error with EAL initialization" << std::endl;
            return -1;
        }

        return 0;
    }

    /**
     * @brief 打印帮助信息
     * 
     * @param program_name 程序名
     */
    static void print_help(const char* program_name) {
        std::cout << "Usage: " << program_name << " [options]" << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  --use-huge          Enable hugepages (default: disabled)" << std::endl;
        std::cout << "  --use-pci           Enable PCI device scanning (default: disabled)" << std::endl;
        std::cout << "  --file-prefix <arg> File prefix for DPDK (default: test)" << std::endl;
        std::cout << "  -m <arg>            Memory to allocate in MB (default: 512)" << std::endl;
        std::cout << "  -n <arg>            Number of memory channels (default: 4)" << std::endl;
        std::cout << "  --help              Show this help message" << std::endl;
        std::cout << "  --<eal-arg>         Pass additional arguments to DPDK EAL" << std::endl;
    }
};

#endif // DPDK_TEST_COMMON_H
