cmake_minimum_required(VERSION 3.20)

# 加载工具链配置
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/arm-none-eabi.cmake)

project(stm32_project C ASM)

add_subdirectory(kernel)
add_subdirectory(drivers)
add_subdirectory(modules)
add_subdirectory(components)
# add_subdirectory(app)
add_subdirectory(app_f407)
# add_subdirectory(bootloader)
# add_subdirectory(app_can)