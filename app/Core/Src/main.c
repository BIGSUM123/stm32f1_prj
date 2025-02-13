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
#include "log_port.h"
#include <stdint.h>
#include "stm32f1xx_ll_rcc.h"
#include "stm32f1xx_ll_system.h"
#include "stm32f1xx_ll_utils.h"
#include "stm32f1xx_ll_cortex.h"
#include "kernel.h"

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

// __attribute__((section(".device_test"), used))
// static struct device test_dev = {
//     .name = "test_device",
//     .api = (void *)0,
// };

// /* USER CODE END 0 */

// extern const struct device _test_device_start[];
// extern const struct device _test_device_end[];

// void device_init_all(void)
// {
//     const struct device *dev;
    
//     for (dev = _test_device_start; dev < _test_device_end; dev++) {
//         // 在这里可以调用设备的初始化函数
//         LOG_INFO("Found device: %s\n", dev->name);
//     }
// }

void rtos_yield(void)
{
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
    __DSB();
    __ISB();
}

void thread1(void);
void thread2(void);

// 使用 __attribute__((aligned(8))) 强制栈数组 8 字节对齐
uint32_t thread1_stack[256] __attribute__((aligned(8)));
uint32_t thread2_stack[256] __attribute__((aligned(8)));

tcb_t tcb1 = {
    .stack_ptr = thread1_stack,
};

tcb_t tcb2 = {
    .stack_ptr = thread2_stack,
};

tcb_t *pxCurrentTCB;      // 当前任务指针

void w_scheduler()
{
    if (pxCurrentTCB == &tcb1)
        pxCurrentTCB = &tcb2;
    else
        pxCurrentTCB = &tcb1;
}

// 栈初始化工具函数
void thread_stack_init(tcb_t *tcb, void (*entry)(void)) {
    // 栈起始地址（确保 8 字节对齐）
    uint32_t *stack_start = (uint32_t*)((uint32_t)tcb->stack_ptr & ~0x7);
    
    // 栈顶指针指向数组末尾（最高地址）
    uint32_t *sp = stack_start + (1024 / sizeof(uint32_t)); // 0x200008A0
    
    // 预留硬件自动保存的 8 个字空间（向下生长）
    sp -= 8;  // 此时 sp = 0x20000880
    
    // 按 Cortex-M 压栈顺序初始化（高地址 → 低地址）
    sp[7] = 0x01000000U;      // xPSR (Thumb 模式)
    sp[6] = (uint32_t)entry;   // PC (线程入口地址)
    sp[5] = 0xFFFFFFFFU;      // LR (无效返回地址)
    sp[4] = 0x00000000U;      // R12
    sp[3] = 0x00000000U;      // R3
    sp[2] = 0x00000000U;      // R2
    sp[1] = 0x00000000U;      // R1
    sp[0] = 0x00000000U;      // R0
    
    // 更新 TCB 中的栈指针（指向硬件帧起始地址）
    tcb->stack_ptr = sp;  // 0x20000880
}
// void thread_stack_init(tcb_t *tcb, void (*entry)(void)) {
//     uint32_t stack_size_words = 1024 / 4;   // 栈大小：256 个字
//     // 让 sp 指向堆栈区域中预留出 8 个字的区域——即顶部的 8 个字
//     // 注意：栈是向下生长的，所以数组末尾存放的是栈底（低地址），而我们希望 PSP 指向这 8 个字的起始地址。
//     uint32_t *sp = &tcb->stack_ptr[stack_size_words - 8];

//     // 按照硬件自动压栈后的内存布局（从 PSP 开始的连续 8 个字）写入：
//     sp[0] = 0x00000000U;       // R0
//     sp[1] = 0x00000000U;       // R1
//     sp[2] = 0x00000000U;       // R2
//     sp[3] = 0x00000000U;       // R3
//     sp[4] = 0x00000000U;       // R12
//     sp[5] = 0xFFFFFFFFU;       // LR（无效返回地址）
//     sp[6] = (uint32_t)entry;   // PC（线程入口地址）
//     sp[7] = 0x01000000U;       // xPSR（Thumb位必须为1）

//     // 更新 TCB 中保存的栈指针，即 PSP 将指向这个区域的起始地址
//     tcb->stack_ptr = sp;
// }

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

    // device_init_all();

    cli_init();
    cli_register_basic_commands();

    // 初始化 PendSV 优先级
    NVIC_SetPriority(PendSV_IRQn, 0xFF);

    // 初始化线程栈
    thread_stack_init(&tcb1, thread1);
    thread_stack_init(&tcb2, thread2);

    pxCurrentTCB = &tcb1;

    // 启用全局中断
    __enable_irq();

    // 触发 SVC 中断来进行首次上下文切换
    __asm("SVC #0");  // 调用 SVC 指令触发 SVC 异常
    // // 触发首次上下文切换（手动跳转到线程1）
    // extern void start_first_thread();
    // start_first_thread();

    while (1);
}


void thread1(void)
{
    // 初始化 NVIC 优先级分组
    NVIC_SetPriorityGrouping(0x00000003U);
    SystemClock_Config();
    MX_GPIO_Init();
    log_init();

    cli_init();
    cli_register_basic_commands();

    // 初始化 PendSV 优先级
    NVIC_SetPriority(PendSV_IRQn, 0xFF);

    while (1)
    {
        uint8_t ch;
        if (log_port_receive(&ch) == 1) {
            cli_process_char(ch);
        }
    }
}


void thread2(void)
{
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
