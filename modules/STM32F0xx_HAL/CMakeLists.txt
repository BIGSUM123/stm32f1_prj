add_library(stm32f0_module STATIC
    STM32F0xx_HAL_Driver/Src/stm32f0xx_ll_rcc.c
    STM32F0xx_HAL_Driver/Src/stm32f0xx_ll_gpio.c
    STM32F0xx_HAL_Driver/Src/stm32f0xx_ll_usart.c
)

target_include_directories(stm32f0_module PUBLIC
    STM32F0xx_HAL_Driver/Inc
    STM32F0xx_HAL_Driver/Inc/Legacy
    CMSIS/Device/ST/STM32F0xx/Include
    CMSIS/Include
)

target_compile_definitions(stm32f0_module
    PRIVATE
        STM32F042x6
        USE_FULL_LL_DRIVER
)