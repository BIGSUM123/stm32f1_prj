add_library(components STATIC
    log/log.c
    log/log_port_stm32f1.c
    cli/cli.c
    flash/flash.c
)

target_include_directories(components PUBLIC
    include/cli
    include/log
    include/flash
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F1xx/Include
    ${CMAKE_SOURCE_DIR}/Drivers/CMSIS/Include
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc
    ${CMAKE_SOURCE_DIR}/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy
)