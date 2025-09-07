# 项目全局配置文件
# 这个文件可以被用户修改来定制构建

# 默认平台 (如果命令行没有指定)
set(DEFAULT_PLATFORM "STM32F407" CACHE STRING "Default platform when not specified")

# 功能开关
option(BUILD_BOOTLOADER "Build bootloader" OFF)
option(BUILD_TESTS "Build unit tests" OFF)
option(ENABLE_DEBUG_LOG "Enable debug logging" ON)
option(ENABLE_PROFILING "Enable performance profiling" OFF)

# 组件选择
option(USE_CLI "Enable command line interface" ON)
option(USE_LOG "Enable logging system" ON)
option(USE_FLASH_MANAGER "Enable flash management" OFF)

# 优化选项
option(OPTIMIZE_FOR_SIZE "Optimize for code size" OFF)
option(ENABLE_LTO "Enable Link Time Optimization" OFF)

# 调试选项
option(ENABLE_ASSERT "Enable assertions" ON)
option(ENABLE_STACK_CHECK "Enable stack overflow checking" OFF)

# 平台特定选项
if(PLATFORM STREQUAL "STM32F407")
    option(USE_FPU "Use hardware FPU" ON)
    option(USE_DSP "Use DSP instructions" ON)
    option(USE_DMA "Use DMA for high-speed transfers" ON)
elseif(PLATFORM STREQUAL "STM32F103")
    # F103没有FPU和DSP
    set(USE_FPU OFF CACHE BOOL "F103 doesn't have FPU" FORCE)
    set(USE_DSP OFF CACHE BOOL "F103 doesn't have DSP" FORCE)
    option(USE_DMA "Use DMA for high-speed transfers" ON)
endif()

# 根据配置设置编译定义
if(ENABLE_DEBUG_LOG)
    add_compile_definitions(DEBUG_LOG_ENABLED)
endif()

if(USE_FPU AND HAS_FPU)
    add_compile_definitions(USE_HARDWARE_FPU)
endif()

if(USE_DSP AND HAS_DSP)
    add_compile_definitions(USE_DSP_INSTRUCTIONS)
endif()

# 打印配置摘要
message(STATUS "Project Configuration:")
message(STATUS "  Platform: ${PLATFORM}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  CLI: ${USE_CLI}")
message(STATUS "  Logging: ${USE_LOG}")
message(STATUS "  FPU: ${USE_FPU}")
message(STATUS "  DSP: ${USE_DSP}")
message(STATUS "  Debug Log: ${ENABLE_DEBUG_LOG}")