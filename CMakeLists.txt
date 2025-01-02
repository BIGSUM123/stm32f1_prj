cmake_minimum_required(VERSION 3.20)

# 加载工具链配置
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/arm-none-eabi.cmake)

project(stm32_project C ASM)

# 添加编译宏定义
add_compile_definitions(
    STM32F103x6
    USE_FULL_LL_DRIVER
)

add_subdirectory(Drivers)
add_subdirectory(components)
add_subdirectory(app)
add_subdirectory(bootloader)