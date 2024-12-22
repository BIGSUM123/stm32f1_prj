#include "log_port.h"
#include <stddef.h>
#include <stdint.h>
#include "stm32f1xx.h"
#include "stm32f1xx_ll_usart.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_bus.h"

#define LOG_UART USART1

int log_port_init(void)
{
    LL_USART_InitTypeDef USART_InitStruct;
    LL_USART_ClockInitTypeDef USART_ClockInitStruct;

    // 使能时钟
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);

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

    LL_USART_Disable(LOG_UART);

    LL_USART_ClockStructInit(&USART_ClockInitStruct);
    if (LL_USART_ClockInit(LOG_UART, &USART_ClockInitStruct) != SUCCESS) {
        return -1;
    }

    LL_USART_StructInit(&USART_InitStruct);
    USART_InitStruct.BaudRate = 115200;
    USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity = LL_USART_PARITY_NONE;
    USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
    if (LL_USART_Init(LOG_UART, &USART_InitStruct) != SUCCESS) {
        return -1;
    }

    LL_USART_Enable(LOG_UART);

    return 0;
}

void log_port_write(const char *data, uint16_t len)
{
    if (data == NULL || len == 0) {
        return;
    }

    for (int i = 0; i < len; i++) {
        while (LL_USART_IsActiveFlag_TXE(LOG_UART) != SET);
        LL_USART_TransmitData8(LOG_UART,data[i]);
    }
}

uint32_t log_port_get_time(void)
{
    // return HAL_GetTick();
    return 1;
}

int log_port_receive(uint8_t *data)
{
    if (data == NULL) {
        return -1;
    }

    while (LL_USART_IsActiveFlag_TXE(LOG_UART) != SET);

    if (LL_USART_IsActiveFlag_RXNE(LOG_UART) == SET) {
        *data = LL_USART_ReceiveData8(LOG_UART);
        return 1;
    }

    return 0;
}
