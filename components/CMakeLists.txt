add_library(components STATIC
    log/log.c
    log/log_port_stm32f1.c
    cli/cli.c
    flash/flash.c
)

target_compile_definitions(components
    PRIVATE
        STM32F103x6
        USE_FULL_LL_DRIVER
)

target_include_directories(components PUBLIC
    include/cli
    include/log
    include/flash
    ${CMAKE_SOURCE_DIR}/modules/STM32F1xx_HAL/CMSIS/Device/ST/STM32F1xx/Include
    ${CMAKE_SOURCE_DIR}/modules/STM32F1xx_HAL/CMSIS/Include
    ${CMAKE_SOURCE_DIR}/modules/STM32F1xx_HAL/STM32F1xx_HAL_Driver/Inc
    ${CMAKE_SOURCE_DIR}/modules/STM32F1xx_HAL/STM32F1xx_HAL_Driver/Inc/Legacy
)