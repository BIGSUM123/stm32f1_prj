# 混合配置系统 - 同时支持CMake变量和Kconfig
# 这个文件展示了如何在不破坏现有系统的情况下添加Kconfig支持

# 检查是否存在Kconfig生成的配置
set(KCONFIG_GENERATED_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/kconfig_generated.cmake")

if(EXISTS ${KCONFIG_GENERATED_FILE})
    # 如果存在Kconfig配置，优先使用
    message(STATUS "Using Kconfig configuration")
    include(${KCONFIG_GENERATED_FILE})
    set(CONFIG_SOURCE "Kconfig")
else()
    # 否则使用原有的CMake配置
    message(STATUS "Using CMake variable configuration")
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/platform_config.cmake)
    set(CONFIG_SOURCE "CMake")
endif()

# 生成配置头文件 (类似于Linux内核的autoconf.h)
set(CONFIG_HEADER_FILE "${CMAKE_BINARY_DIR}/include/generated/config.h")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/include/generated")

# 开始生成配置头文件
file(WRITE ${CONFIG_HEADER_FILE} "/* Auto-generated configuration header */\n")
file(APPEND ${CONFIG_HEADER_FILE} "/* Generated from: ${CONFIG_SOURCE} */\n")
file(APPEND ${CONFIG_HEADER_FILE} "#ifndef CONFIG_H\n")
file(APPEND ${CONFIG_HEADER_FILE} "#define CONFIG_H\n\n")

# 平台配置
if(PLATFORM STREQUAL "STM32F103")
    file(APPEND ${CONFIG_HEADER_FILE} "#define CONFIG_PLATFORM_STM32F103 1\n")
    file(APPEND ${CONFIG_HEADER_FILE} "#define CONFIG_ARM_CORTEX_M3 1\n")
elseif(PLATFORM STREQUAL "STM32F407")
    file(APPEND ${CONFIG_HEADER_FILE} "#define CONFIG_PLATFORM_STM32F407 1\n")
    file(APPEND ${CONFIG_HEADER_FILE} "#define CONFIG_ARM_CORTEX_M4 1\n")
endif()

# 硬件特性配置
if(HAS_FPU)
    file(APPEND ${CONFIG_HEADER_FILE} "#define CONFIG_HAS_FPU 1\n")
endif()

if(HAS_DSP)
    file(APPEND ${CONFIG_HEADER_FILE} "#define CONFIG_HAS_DSP 1\n")
endif()

# 组件配置
if(USE_CLI)
    file(APPEND ${CONFIG_HEADER_FILE} "#define CONFIG_USE_CLI 1\n")
endif()

if(USE_LOG)
    file(APPEND ${CONFIG_HEADER_FILE} "#define CONFIG_USE_LOG 1\n")
endif()

# 结束头文件
file(APPEND ${CONFIG_HEADER_FILE} "\n#endif /* CONFIG_H */\n")

# 添加生成的头文件路径到包含目录
include_directories("${CMAKE_BINARY_DIR}/include/generated")

message(STATUS "Generated configuration header: ${CONFIG_HEADER_FILE}")

# 打印配置摘要
message(STATUS "Configuration Summary (${CONFIG_SOURCE}):")
message(STATUS "  Platform: ${PLATFORM}")
message(STATUS "  MCU Family: ${MCU_FAMILY}")
message(STATUS "  ARM Core: ${ARM_CORE}")
message(STATUS "  FPU: ${HAS_FPU}")
message(STATUS "  DSP: ${HAS_DSP}")

# 为了演示，创建一个配置验证函数
function(validate_configuration)
    # 检查配置的一致性
    if(PLATFORM STREQUAL "STM32F103" AND USE_FPU)
        message(WARNING "STM32F103 doesn't have FPU, disabling USE_FPU")
        set(USE_FPU OFF PARENT_SCOPE)
    endif()
    
    if(PLATFORM STREQUAL "STM32F103" AND USE_DSP)
        message(WARNING "STM32F103 doesn't have DSP, disabling USE_DSP")
        set(USE_DSP OFF PARENT_SCOPE)
    endif()
endfunction()

# 运行配置验证
validate_configuration()