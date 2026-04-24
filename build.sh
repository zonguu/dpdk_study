#!/bin/bash

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 解析命令行参数
ENABLE_ASAN=OFF
ENABLE_TESTS=ON
CLEAN_BUILD=OFF
JOBS=4
BUILD=0
COMPILE=0

# 打印带颜色的消息
print_info()
{
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_error()
{
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning()
{
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

function configure_project()
{
    print_info "Configuring CMake with ASAN=$ENABLE_ASAN, TESTS=$ENABLE_TESTS..."
    cd "$BUILD_DIR" || exit 1
    cmake -DENABLE_ASAN="$ENABLE_ASAN" -DENABLE_TESTS="$ENABLE_TESTS" .. || {
        print_error "CMake configuration failed!"
        exit 1
    }
}

function build_project()
{
    print_info "Building project with $JOBS jobs..."
    # 确保在构建目录中
    if [ ! -f "CMakeCache.txt" ]; then
        print_warning "CMakeCache.txt not found. Running configuration first..."
        configure_project
    fi
    cmake --build . -- -j"$JOBS" || {
        print_error "Build failed!"
        exit 1
    }
}

# 显示使用说明
usage()
{
    echo "Usage: $0 [OPTIONS]"
    echo "Options:"
    echo "  --asan=1           Enable AddressSanitizer 	(default: OFF)"
    echo "  --test=1           Enable tests 			(default: ON)"
    echo "  -j=NUM             Number of parallel jobs 	(default: 4)"
    echo "  -b                 Configure project (CMake)"
    echo "  -c                 Build project (Compile)"
    echo "  --clean            Clean build directory before building"
    echo "  --help, -h         Show this help message"
}

# 参数解析
while [[ $# -gt 0 ]]; do
    case "$1" in
        -b)
            BUILD=1
            shift
            ;;
        -c)
            COMPILE=1
            shift
            ;;
        -j=*)
            JOBS="${1#*=}"
            shift
            ;;
        --asan=*)
            ENABLE_ASAN="${1#*=}"
            shift
            ;;
        --test=*)
            ENABLE_TESTS="${1#*=}"
            shift
            ;;
        --clean)
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
done # <--- 修复：添加了 done

# 设置构建目录
BUILD_DIR=build

# 清理构建目录
if [ "$CLEAN_BUILD" = "ON" ]; then
    print_info "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

# 创建构建目录
if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
fi

# 逻辑优化：如果选择了编译，通常也需要先配置
if [ "$COMPILE" = "1" ]; then
    # 如果用户没有单独指定配置，但在编译时发现需要配置，则自动处理
    # 这里为了逻辑清晰，如果选了编译，我们强制执行配置步骤
    BUILD=1
fi

# 执行配置
if [ "$BUILD" = "1" ]; then
    configure_project
fi

# 执行编译
if [ "$COMPILE" = "1" ]; then
    build_project
fi

# 只有当执行了实际操作时才提示成功
if [ "$BUILD" = "1" ] || [ "$COMPILE" = "1" ]; then
    print_info "Operations completed successfully!"
else
    print_warning "No build or compile actions specified. Use -b or -c."
    usage
fi