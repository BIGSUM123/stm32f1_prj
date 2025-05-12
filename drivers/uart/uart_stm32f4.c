#include <stdint.h>
#include "drivers/uart.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_bus.h"
#include "uart_stm32f4.h"

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

static int uart_stm32_configure(const struct device *dev, const struct uart_config *cfg)
{
    const struct uart_stm32_config *config = dev->config;
    USART_TypeDef *uart = config->usart;

    LL_USART_InitTypeDef USART_InitStruct;
    LL_USART_ClockInitTypeDef USART_ClockInitStruct;
    
    // LL_USART_DeInit(uart);

    // 使能时钟
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);

    LL_GPIO_InitTypeDef GPIO_InitStruct;
    LL_GPIO_StructInit(&GPIO_InitStruct);

    // PA9 - TX
    GPIO_InitStruct.Pin = LL_GPIO_PIN_9;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // PA10 - RX
    GPIO_InitStruct.Pin = LL_GPIO_PIN_10;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;    // 添加上拉，避免悬空
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    LL_USART_ClockStructInit(&USART_ClockInitStruct);
    if (LL_USART_ClockInit(uart, &USART_ClockInitStruct) != SUCCESS) {
        return -1;
    }

    LL_USART_Disable(uart);

    LL_USART_StructInit(&USART_InitStruct);
    USART_InitStruct.BaudRate = 115200;
    USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity = LL_USART_PARITY_NONE;
    USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
    if (LL_USART_Init(uart, &USART_InitStruct) != SUCCESS) {
        return -1;
    }

    LL_USART_Enable(uart);

    return 0;
}

static int uart_stm32_init(const struct device *dev)
{
    // const struct uart_stm32_config *config = dev->config;
    // LL_USART_InitTypeDef uart_init;

    // LL_USART_ClockInitTypeDef USART_ClockInitStruct;
    // LL_GPIO_InitTypeDef GPIO_InitStruct;
    // LL_GPIO_StructInit(&GPIO_InitStruct);

    // LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);

    // // PA9 - TX
    // GPIO_InitStruct.Pin = LL_GPIO_PIN_9;
    // GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    // GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    // GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    // LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // // PA10 - RX
    // GPIO_InitStruct.Pin = LL_GPIO_PIN_10;
    // GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    // GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;    // 添加上拉，避免悬空
    // LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // LL_USART_ClockStructInit(&USART_ClockInitStruct);
    // if (LL_USART_ClockInit(config->usart, &USART_ClockInitStruct) != SUCCESS) {
    //     return -1;
    // }

    // LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);

    // uart_init.BaudRate = STM32_UART_DEFAULT_BAUDRATE;
    // uart_init.DataWidth = LL_USART_DATAWIDTH_8B;
    // uart_init.StopBits = LL_USART_STOPBITS_1;
    // uart_init.Parity = LL_USART_PARITY_NONE;
    // uart_init.TransferDirection = LL_USART_DIRECTION_TX_RX;
    // uart_init.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    // uart_init.OverSampling = LL_USART_OVERSAMPLING_16;

    // LL_USART_Disable(config->usart);
    // LL_USART_Init(config->usart, &uart_init);
    // LL_USART_Enable(config->usart);

    return 0;
}

static const uart_driver_api_t uart_stm32_api = {
    .poll_in = uart_stm32_poll_in,
    .poll_out = uart_stm32_poll_out,
    .configure = uart_stm32_configure,
    .config_get = NULL,
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
