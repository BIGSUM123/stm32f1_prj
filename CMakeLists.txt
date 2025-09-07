cmake_minimum_required(VERSION 3.20)

# 加载平台配置 (必须在project之前)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/platform_config.cmake)

# 加载工具链配置
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/arm-none-eabi.cmake)

project(stm32_project C ASM)

# 通用模块 (所有平台都需要)
add_subdirectory(kernel)
add_subdirectory(drivers)
add_subdirectory(components)

# 平台相关的HAL模块
add_subdirectory(modules)

# 根据平台选择应用
if(PLATFORM STREQUAL "STM32F103")
    add_subdirectory(app)
elseif(PLATFORM STREQUAL "STM32F407")
    add_subdirectory(app_f407)
endif()

# 可选的bootloader
if(BUILD_BOOTLOADER)
    add_subdirectory(bootloader)
endif()