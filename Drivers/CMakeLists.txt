add_library(stm32f1_drivers STATIC
    STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_rcc.c
    STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_gpio.c
    STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_usart.c
)

target_include_directories(stm32f1_drivers PUBLIC
    STM32F1xx_HAL_Driver/Inc
    STM32F1xx_HAL_Driver/Inc/Legacy
    CMSIS/Device/ST/STM32F1xx/Include
    CMSIS/Include
)