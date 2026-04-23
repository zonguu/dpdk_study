#!/bin/bash

# 颜色定义
RED='[0;31m'
GREEN='[0;32m'
YELLOW='[1;33m'
NC='[0m' # No Color

# 打印带颜色的消息
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# 显示使用说明
usage() {
    echo "Usage: $0 [OPTIONS]"
    echo "Options:"
    echo "  --asan, -a          Enable AddressSanitizer"
    echo "  --no-tests, -n      Disable tests"
    echo "  --clean, -c         Clean build directory before building"
    echo "  --help, -h          Show this help message"
}

# 解析命令行参数
ENABLE_ASAN=OFF
ENABLE_TESTS=ON
CLEAN_BUILD=OFF

while [[ $# -gt 0 ]]; do
    case "$1" in
        --asan|-a)
            ENABLE_ASAN=ON
            shift
            ;;
        --no-tests|-n)
            ENABLE_TESTS=OFF
            shift
            ;;
        --clean|-c)
            CLEAN_BUILD=ON
            shift
            ;;
        --help|-h)
            usage
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
done

# 设置构建目录
BUILD_DIR=build

# 清理构建目录
if [ "$CLEAN_BUILD" = "ON" ]; then
    print_info "Cleaning build directory..."
    rm -rf $BUILD_DIR
fi

# 创建构建目录
if [ ! -d "$BUILD_DIR" ]; then
    print_info "Creating build directory..."
    mkdir -p $BUILD_DIR
fi

# 配置CMake
print_info "Configuring CMake with ASAN=$ENABLE_ASAN, TESTS=$ENABLE_TESTS..."
cd $BUILD_DIR
cmake -DENABLE_ASAN=$ENABLE_ASAN -DENABLE_TESTS=$ENABLE_TESTS .. || {
    print_error "CMake configuration failed!"
    exit 1
}

# 构建项目
print_info "Building project..."
make -j$(nproc) || {
    print_error "Build failed!"
    exit 1
}

# 运行测试
if [ "$ENABLE_TESTS" = "ON" ]; then
    print_info "Running tests..."
    ctest --output-on-failure || {
        print_error "Tests failed!"
        exit 1
    }
    print_info "All tests passed!"
fi

print_info "Build completed successfully!"
