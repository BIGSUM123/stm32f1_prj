#include <stdint.h>
#include "drivers/uart.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx.h" // 添加寄存器定义
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

	// 只等待发送缓冲区空，不等待发送完成
	while (!LL_USART_IsActiveFlag_TXE(uart)) {
	}

	// 发送数据
	LL_USART_TransmitData8(uart, out_char);
	
	// 注意：不等待TC标志，这样可以提高发送效率
	// TC标志会在下一个字符发送前自动检查TXE时确保完成
}

static int uart_stm32_configure(const struct device *dev, const struct uart_config *cfg)
{
	const struct uart_stm32_config *config = dev->config;
	USART_TypeDef *uart = config->usart;

	// 使能时钟
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;  // GPIOA时钟
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN; // USART1时钟

	// 配置GPIO - PA9(TX), PA10(RX)
	// 设置为复用功能模式
	GPIOA->MODER &= ~(GPIO_MODER_MODER9 | GPIO_MODER_MODER10);
	GPIOA->MODER |= (GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1);

	// 推挽输出，高速
	GPIOA->OTYPER &= ~(GPIO_OTYPER_OT_9 | GPIO_OTYPER_OT_10);
	GPIOA->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR9 | GPIO_OSPEEDER_OSPEEDR10);

	// RX上拉，TX无上下拉
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR9 | GPIO_PUPDR_PUPDR10);
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR10_0; // RX上拉

	// 设置复用功能 AF7 (USART1)
	// PA9 -> AFR[1] bits 4-7, PA10 -> AFR[1] bits 8-11
	GPIOA->AFR[1] &= ~((0xF << 4) | (0xF << 8)); // 清除PA9和PA10的AF位
	GPIOA->AFR[1] |= (7 << 4) | (7 << 8);        // 设置AF7

	// 配置USART1
	uart->CR1 = 0; // 复位所有控制位
	uart->CR2 = 0;
	uart->CR3 = 0;

	// 波特率计算: BRR = APB2_CLK / BAUDRATE
	// APB2 = 84MHz, BAUDRATE = 115200
	// BRR = 84000000 / 115200 = 729.17 ≈ 729
	uart->BRR = 729;

	// 使能发送器、接收器和USART
	uart->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;

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

#define UART_DEVICE_DEFINE(__suffix, __SUFFIX)                                                     \
	static const struct uart_stm32_config uart_stm32_cfg_##__suffix = {                        \
		.usart = __SUFFIX,                                                                 \
	};                                                                                         \
	static struct uart_stm32_data uart_stm32_data_##__suffix;                                  \
	static device_state_t uart_stm32_state_##__suffix;                                         \
	DEVICE_DEFINE(__suffix, #__suffix, &uart_stm32_init, NULL, &uart_stm32_data_##__suffix,    \
		      &uart_stm32_cfg_##__suffix, 1, 1, &uart_stm32_api,                           \
		      &uart_stm32_state_##__suffix)

#define UART_DEVICE_DEFINE_STM32(__suffix, __SUFFIX)                                               \
	UART_DEVICE_DEFINE(uart##__suffix, USART##__SUFFIX)

UART_DEVICE_DEFINE_STM32(1, 1);
