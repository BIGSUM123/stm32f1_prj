target_sources(drivers PRIVATE
    uart_stm32f1.c
)

target_include_directories(drivers PRIVATE
    PRIVATE
        ${CMAKE_SOURCE_DIR}/modules/STM32F1xx_HAL/CMSIS/Include
        ${CMAKE_SOURCE_DIR}/modules/STM32F1xx_HAL/CMSIS/Device/ST/STM32F1xx/Include
        ${CMAKE_SOURCE_DIR}/modules/STM32F1xx_HAL/STM32F1xx_HAL_Driver/Inc
        ${CMAKE_SOURCE_DIR}/modules/STM32F1xx_HAL/STM32F1xx_HAL_Driver/Inc/Legacy
)

target_compile_definitions(drivers PRIVATE
    STM32F103x6
    USE_FULL_LL_DRIVER
)
