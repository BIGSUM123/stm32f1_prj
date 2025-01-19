target_sources(drivers
    PRIVATE
        gpio_stm32f1.c
)

# STM32 相关的包含路径
target_include_directories(drivers
    PRIVATE
        ${CMAKE_SOURCE_DIR}/modules/CMSIS/Device/ST/STM32F1xx/Include
        ${CMAKE_SOURCE_DIR}/modules/CMSIS/Include
        ${CMAKE_SOURCE_DIR}/modules/STM32F1xx_HAL_Driver/Inc
        ${CMAKE_SOURCE_DIR}/modules/STM32F1xx_HAL_Driver/Inc/Legacy
)

# STM32 相关的编译定义
target_compile_definitions(drivers
    PRIVATE
        STM32F103x6
        USE_FULL_LL_DRIVER
)