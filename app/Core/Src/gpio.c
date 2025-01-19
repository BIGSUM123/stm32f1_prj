/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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
#include "gpio.h"
#include "stm32f103x6.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_bus.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{
  LL_GPIO_InitTypeDef led_init;

  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOC);

  led_init.Pin = LL_GPIO_PIN_13;
  led_init.Mode = LL_GPIO_MODE_OUTPUT;
  led_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  led_init.Speed = LL_GPIO_SPEED_FREQ_LOW;
  LL_GPIO_Init(GPIOC, &led_init);

  LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_13);
}

/* USER CODE BEGIN 2 */

int led_ctrl(led_state state)
{
  switch (state) {
  case LED_OFF:
    LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_13);
    break;

  case LED_ON:
    LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_13);
    break;

  default:
    return -1;
    break;
  }

  return 0;
}

/* USER CODE END 2 */
