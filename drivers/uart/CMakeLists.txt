target_sources(drivers PRIVATE
    # uart_stm32f1.c
    uart_stm32f4.c
)

target_include_directories(drivers PRIVATE
    PRIVATE
        ${CMAKE_SOURCE_DIR}/modules/STM32F4xx_HAL/CMSIS/Include
        ${CMAKE_SOURCE_DIR}/modules/STM32F4xx_HAL/CMSIS/Device/ST/STM32F4xx/Include
        ${CMAKE_SOURCE_DIR}/modules/STM32F4xx_HAL/STM32F4xx_HAL_Driver/Inc
        ${CMAKE_SOURCE_DIR}/modules/STM32F4xx_HAL/STM32F4xx_HAL_Driver/Inc/Legacy
)

target_compile_definitions(drivers PRIVATE
    STM32F407xx
    USE_FULL_LL_DRIVER
)
