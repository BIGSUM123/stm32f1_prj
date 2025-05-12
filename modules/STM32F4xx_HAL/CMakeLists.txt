add_library(stm32f4_module STATIC
    STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_rcc.c
    STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_gpio.c
    STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usart.c
    STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_utils.c
)

target_include_directories(stm32f4_module PUBLIC
    STM32F4xx_HAL_Driver/Inc
    STM32F4xx_HAL_Driver/Inc/Legacy
    CMSIS/Device/ST/STM32F4xx/Include
    CMSIS/Include
)

target_compile_definitions(stm32f4_module
    PRIVATE
        STM32F407xx
        USE_FULL_LL_DRIVER
)