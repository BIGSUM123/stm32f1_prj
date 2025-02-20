#include <stdint.h>
#include "drivers/uart.h"
#include "stm32f1xx_ll_usart.h"
#include "stm32f1xx_ll_bus.h"
#include "uart_stm32f1.h"

static int uart_stm32_poll_in(const struct device *dev, uint8_t *p_char)
{
    const struct uart_stm32_config *config = dev->config;
    USART_TypeDef *uart = config->usart;

    if (!LL_USART_IsActiveFlag_RXNE(uart)) {
        return -1;
    }

    *p_char = LL_USART_ReceiveData8(uart);
    return 0;
}

static void uart_stm32_poll_out(const struct device *dev, uint8_t out_char)
{
    const struct uart_stm32_config *config = dev->config;
    USART_TypeDef *uart = config->usart;

    while (!LL_USART_IsActiveFlag_TXE(uart)) {
    }

    LL_USART_TransmitData8(uart, out_char);
}

static int uart_stm32_init(const struct device *dev)
{
    const struct uart_stm32_config *config = dev->config;

    LL_USART_InitTypeDef uart_init;

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);

    uart_init.BaudRate = STM32_UART_DEFAULT_BAUDRATE;
    uart_init.DataWidth = LL_USART_DATAWIDTH_8B;
    uart_init.StopBits = LL_USART_STOPBITS_1;
    uart_init.Parity = LL_USART_PARITY_NONE;
    uart_init.TransferDirection = LL_USART_DIRECTION_TX_RX;
    uart_init.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    uart_init.OverSampling = LL_USART_OVERSAMPLING_16;

    LL_USART_Init(config->usart, &uart_init);
    LL_USART_Enable(config->usart);

    return 0;
}

static const uart_driver_api_t uart_stm32_api = {
    .poll_in = uart_stm32_poll_in,
    .poll_out = uart_stm32_poll_out,
};

#define UART_DEVICE_DEFINE(__suffix, __SUFFIX)                    \
    static const struct uart_stm32_config uart_stm32_cfg_##__suffix = {  \
        .usart = __SUFFIX,                                        \
    };                                                           \
    static struct uart_stm32_data uart_stm32_data_##__suffix;    \
    static device_state_t uart_stm32_state_##__suffix;           \
    DEVICE_DEFINE(                                               \
        __suffix, #__suffix,                                     \
        &uart_stm32_init,                                        \
        NULL,                                                    \
        &uart_stm32_data_##__suffix,                            \
        &uart_stm32_cfg_##__suffix,                             \
        1, 1,                                                    \
        &uart_stm32_api,                                         \
        &uart_stm32_state_##__suffix                            \
    )

#define UART_DEVICE_DEFINE_STM32(__suffix, __SUFFIX)             \
    UART_DEVICE_DEFINE(uart##__suffix, USART##__SUFFIX)

UART_DEVICE_DEFINE_STM32(1, 1);
