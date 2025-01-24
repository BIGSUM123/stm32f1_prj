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
#include "boot.h"
#include "cli.h"
#include "cli_commands.h"
#include "gpio.h"
#include "log.h"
#include "log_port.h"
#include <stdint.h>
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_system.h"
#include "stm32f1xx_ll_utils.h"
#include "stm32f1xx_ll_cortex.h"

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

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    // 初始化 NVIC 优先级分组
    NVIC_SetPriorityGrouping(0x00000003U);
    SystemClock_Config();
    MX_GPIO_Init();
    log_init();

    if (boot_check_app() == 0) {
        LOG_INFO("Jumping to application...");
        boot_jump_to_app();
    } else {
        LOG_INFO(" App Status: Invaild");
    }

    cli_init();
    cli_register_basic_commands();

    while (1)
    {
        uint8_t ch;
        if (log_port_receive(&ch) == 1) {
            cli_process_char(ch);
        }
    }
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    // 配置 Flash 等待周期和预取缓冲
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
    LL_FLASH_EnablePrefetch();

    // 使能 HSI
    LL_RCC_HSI_Enable();
    while(LL_RCC_HSI_IsReady() != 1);

    // 配置 PLL (HSI/2 * 16 = 72MHz)
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI_DIV_2, LL_RCC_PLL_MUL_16);
    
    // 使能 PLL
    LL_RCC_PLL_Enable();
    while(LL_RCC_PLL_IsReady() != 1);

    // 设置系统分频
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2); 
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
    
    // 设置系统时钟源为 PLL
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);

    // 配置 SysTick
    SystemCoreClockUpdate();
    LL_InitTick(72000000, 1000U);  // 1ms 的 SysTick 中断
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
    while (1)
    {
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
