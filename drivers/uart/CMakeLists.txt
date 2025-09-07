# 根据平台自动选择UART驱动
if(MCU_FAMILY STREQUAL "STM32F1")
    target_sources(drivers PRIVATE uart_stm32f1.c)
elseif(MCU_FAMILY STREQUAL "STM32F4")
    target_sources(drivers PRIVATE uart_stm32f4.c)
elseif(MCU_FAMILY STREQUAL "STM32F7")
    target_sources(drivers PRIVATE uart_stm32f7.c)  # 未来扩展
endif()

# 使用平台配置的包含路径和定义
target_include_directories(drivers PRIVATE ${PLATFORM_INCLUDES})
target_compile_definitions(drivers PRIVATE ${PLATFORM_DEFINES})
