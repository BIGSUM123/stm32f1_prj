/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cli.h"
#include "cli_commands.h"
#include "gpio.h"
#include "log.h"
#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_pwr.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_utils.h"
#include "stm32f4xx_ll_cortex.h"
#include "kernel.h"
#include "rtos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* RTOS Test Tasks */
void led_task(void *parameter);
void cli_task(void *parameter);

/* Task handles */
static task_handle_t led_task_handle = NULL;
static task_handle_t cli_task_handle = NULL;

uint32_t uart_init_ret = 0;

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
	SystemInit();

	// 初始化 NVIC 优先级分组
	NVIC_SetPriorityGrouping(0x00000003U);
	SystemClock_Config();

	led_init();
	uart_init_ret = log_init();

	// 添加调试信息 - 检查时钟配置
	volatile uint32_t sysclk = SystemCoreClock; // 应该是168MHz
	volatile uint32_t apb2_div = (RCC->CFGR & RCC_CFGR_PPRE2) >> RCC_CFGR_PPRE2_Pos;
	volatile uint32_t apb2_clk = (apb2_div < 4) ? sysclk : sysclk / (1 << (apb2_div - 3));
	volatile uint32_t usart_brr = USART1->BRR;

	(void)sysclk;
	(void)apb2_clk;
	(void)usart_brr; // 防止编译器优化

	cli_init();
	cli_register_basic_commands();

	LOG_DBG("System initialized, RTOS starting...");

	// Check if RTOS is initialized
	extern bool rtos_is_initialized(void);
	if (!rtos_is_initialized()) {
		LOG_DBG("ERROR: RTOS not initialized!");
		while (1)
			;
	}

	// Set interrupt priorities according to RTOS standards
	// SysTick should have highest priority for accurate timing
	NVIC_SetPriority(SysTick_IRQn, 0x00); // Highest priority

	// PendSV should have lowest priority to avoid blocking other interrupts
	NVIC_SetPriority(PendSV_IRQn, 0xFF); // Lowest priority

	// SVC should also have high priority
	// NVIC_SetPriority(SVCall_IRQn, 0x00); // Highest priority

	// Create RTOS tasks
	led_task_handle = task_create("LED_TASK", led_task, NULL, 512, RTOS_PRIORITY_NORMAL);
	if (led_task_handle == NULL) {
		LOG_DBG("Failed to create LED task");
	} else {
		LOG_DBG("LED task created successfully");
	}

	cli_task_handle = task_create("CLI_TASK", cli_task, NULL, 1024, RTOS_PRIORITY_NORMAL);
	if (cli_task_handle == NULL) {
		LOG_DBG("Failed to create CLI task");
	} else {
		LOG_DBG("CLI task created successfully");
	}

	// Debug: Check system state before starting
	extern uint8_t rtos_get_ready_bitmap(void);
	uint8_t bitmap = rtos_get_ready_bitmap();
	LOG_DBG("Ready bitmap: 0x%02X", bitmap);

	// Debug: Check SystemCoreClock
	extern uint32_t SystemCoreClock;
	LOG_DBG("SystemCoreClock: %lu Hz", SystemCoreClock);
}

/**
 * @brief LED task - blinks LED and demonstrates task switching
 */
void led_task(void *parameter)
{
	(void)parameter; // Unused parameter

	LOG_DBG("LED task started");

	uint32_t counter = 0;

	while (1) {
		counter++;

		led_ctrl(LED_ON);

		// Shorter busy wait to allow task switching
		task_delay_ms(1000);
		led_ctrl(LED_OFF);

		// Shorter busy wait to allow task switching
		task_delay_ms(1000);

		// More frequent debug output and yielding
		if (counter % 2 == 0) {
			// LOG_DBG("LED task running, count: %lu", counter);
		}
	}
}

/**
 * @brief CLI task - handles command line interface
 */
void cli_task(void *parameter)
{
	(void)parameter; // Unused parameter

	LOG_DBG("CLI task started");

	while (1) {
		uint8_t ch;
		if (log_read(&ch) == 1) {
			cli_process_char(ch);
		}
	}
}

/**
 * @brief  System Clock Configuration for STM32F407
 *         System Clock source            = PLL (HSE)
 *         SYSCLK(Hz)                     = 168000000
 *         HCLK(Hz)                       = 168000000
 *         AHB Prescaler                  = 1
 *         APB1 Prescaler                 = 4 (max 42 MHz)
 *         APB2 Prescaler                 = 2 (max 84 MHz)
 *         PLL_M                          = 8  (HSE = 8MHz)
 *         PLL_N                          = 336
 *         PLL_P                          = 2
 *         PLL_Q                          = 7 (for USB OTG FS)
 */
void SystemClock_Config(void)
{
	// 配置 Flash 等待周期和预取缓冲
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_5);
	LL_FLASH_EnablePrefetch();

	// 1. 开启 HSE
	LL_RCC_HSE_Enable();
	while (LL_RCC_HSE_IsReady() != 1)
		;

	// 2. 设置电源和电压调节（必须）
	// 首先使能PWR时钟
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

	LL_PWR_EnableBkUpAccess();
	LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);

	// 简单延时等待电压稳定（更可靠的方法）
	for (volatile uint32_t i = 0; i < 10000; i++)
		;

	// 3. 开启 PLL 并配置参数
	LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_8, 336,
				    LL_RCC_PLLP_DIV_2); // 8MHz /8 * 336 /2 = 168 MHz
	LL_RCC_PLL_Enable();
	while (LL_RCC_PLL_IsReady() != 1)
		;

	// 4. 设置总线分频器
	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1); // HCLK = 168 MHz
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_4);  // APB1 = 42 MHz
	LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);  // APB2 = 84 MHz

	// 5. 切换系统时钟源为 PLL
	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
	while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
		;

	// 6. 更新系统核心时钟变量
	SystemCoreClockUpdate();

	// 7. 初始化 SysTick 定时器（1ms）
	LL_InitTick(SystemCoreClock, 100);
	LL_SYSTICK_EnableIT();
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	   ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
