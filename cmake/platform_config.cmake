# 平台配置系统

# 支持的平台列表
set(SUPPORTED_PLATFORMS "STM32F103" "STM32F407" "STM32F746")

# 默认平台 (可以通过命令行覆盖)
if(NOT DEFINED PLATFORM)
    set(PLATFORM "STM32F407" CACHE STRING "Target platform")
endif()

# 验证平台是否支持
if(NOT PLATFORM IN_LIST SUPPORTED_PLATFORMS)
    message(FATAL_ERROR "Unsupported platform: ${PLATFORM}. Supported: ${SUPPORTED_PLATFORMS}")
endif()

message(STATUS "Building for platform: ${PLATFORM}")

# 根据平台设置配置
if(PLATFORM STREQUAL "STM32F103")
    # STM32F103配置
    set(MCU_FAMILY "STM32F1")
    set(MCU_SERIES "STM32F103")
    set(MCU_PART "STM32F103C8")
    set(ARM_CORE "cortex-m3")
    set(HAL_MODULE "STM32F1xx_HAL")
    set(HAS_FPU FALSE)
    set(HAS_DSP FALSE)
    set(FLASH_SIZE "64K")
    set(RAM_SIZE "20K")
    
elseif(PLATFORM STREQUAL "STM32F407")
    # STM32F407配置
    set(MCU_FAMILY "STM32F4")
    set(MCU_SERIES "STM32F407")
    set(MCU_PART "STM32F407VE")
    set(ARM_CORE "cortex-m4")
    set(HAL_MODULE "STM32F4xx_HAL")
    set(HAS_FPU TRUE)
    set(HAS_DSP TRUE)
    set(FLASH_SIZE "512K")
    set(RAM_SIZE "192K")
    
elseif(PLATFORM STREQUAL "STM32F746")
    # STM32F746配置 (未来扩展)
    set(MCU_FAMILY "STM32F7")
    set(MCU_SERIES "STM32F746")
    set(MCU_PART "STM32F746NG")
    set(ARM_CORE "cortex-m7")
    set(HAL_MODULE "STM32F7xx_HAL")
    set(HAS_FPU TRUE)
    set(HAS_DSP TRUE)
    set(FLASH_SIZE "1024K")
    set(RAM_SIZE "320K")
endif()

# 设置编译器标志
if(ARM_CORE STREQUAL "cortex-m3")
    set(CPU_FLAGS "-mcpu=cortex-m3 -mthumb")
elseif(ARM_CORE STREQUAL "cortex-m4")
    if(HAS_FPU)
        set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=soft")
    else()
        set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb")
    endif()
elseif(ARM_CORE STREQUAL "cortex-m7")
    set(CPU_FLAGS "-mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=soft")
endif()

# 设置HAL路径
set(HAL_BASE_PATH "${CMAKE_SOURCE_DIR}/modules/${HAL_MODULE}")
set(HAL_CMSIS_PATH "${HAL_BASE_PATH}/CMSIS")
set(HAL_DRIVER_PATH "${HAL_BASE_PATH}/${HAL_MODULE}_Driver")

# 设置编译定义
set(PLATFORM_DEFINES 
    ${MCU_PART}xx
    USE_FULL_LL_DRIVER
)

if(HAS_FPU)
    list(APPEND PLATFORM_DEFINES ARM_MATH_CM4)
endif()

if(HAS_DSP)
    list(APPEND PLATFORM_DEFINES ARM_MATH_DSP)
endif()

# 设置包含路径
set(PLATFORM_INCLUDES
    ${HAL_CMSIS_PATH}/Include
    ${HAL_CMSIS_PATH}/Device/ST/${MCU_FAMILY}xx/Include
    ${HAL_DRIVER_PATH}/Inc
    ${HAL_DRIVER_PATH}/Inc/Legacy
)

# 导出配置供其他模块使用
set(PLATFORM_CONFIG_LOADED TRUE CACHE INTERNAL "Platform config loaded")

# 打印配置信息
message(STATUS "Platform Configuration:")
message(STATUS "  MCU Family: ${MCU_FAMILY}")
message(STATUS "  MCU Series: ${MCU_SERIES}")
message(STATUS "  ARM Core: ${ARM_CORE}")
message(STATUS "  FPU Support: ${HAS_FPU}")
message(STATUS "  DSP Support: ${HAS_DSP}")
message(STATUS "  Flash Size: ${FLASH_SIZE}")
message(STATUS "  RAM Size: ${RAM_SIZE}")