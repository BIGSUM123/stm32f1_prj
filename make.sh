#!/bin/bash

BUILD_TYPE=Release
TARGET=all

# 解析命令行参数
while [ $# -gt 0 ]; do
    case "$1" in
        debug)
            BUILD_TYPE=Debug
            ;;
        release)
            BUILD_TYPE=Release
            ;;
        app)
            TARGET=app
            ;;
        boot)
            TARGET=bootloader
            ;;
        clean)
            if [ -d "build" ]; then
                rm -rf build
            fi
            exit 0
            ;;
    esac
    shift
done

# 创建构建目录并执行构建
mkdir -p build
cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -G Ninja -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ..
ninja ${TARGET}
cd ..