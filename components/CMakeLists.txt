add_library(components STATIC
    log/log.c
    cli/cli.c
    # flash/flash.c
)

target_compile_definitions(components
    PRIVATE
        STM32F407xx
        USE_FULL_LL_DRIVER
)

target_link_libraries(components
    drivers
)

target_include_directories(components PUBLIC
    include/cli
    include/log
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/modules/STM32F4xx_HAL/CMSIS/Device/ST/STM32F4xx/Include
    ${CMAKE_SOURCE_DIR}/modules/STM32F4xx_HAL/CMSIS/Include
    ${CMAKE_SOURCE_DIR}/modules/STM32F4xx_HAL/STM32F4xx_HAL_Driver/Inc
    ${CMAKE_SOURCE_DIR}/modules/STM32F4xx_HAL/STM32F4xx_HAL_Driver/Inc/Legacy
)