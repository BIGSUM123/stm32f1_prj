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
#include "device.h"
#include "drivers/gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */
const struct device *gpioc;
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
  gpioc = device_get_binding("gpioc");
  gpio_pin_configure(gpioc, 13, 0);
}

/* USER CODE BEGIN 2 */

int led_ctrl(led_state state)
{
  switch (state) {
  case LED_OFF:
    // LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_13);
    gpio_pin_set_raw(gpioc, 13, 1);
    break;

  case LED_ON:
    // LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_13);
    gpio_pin_set_raw(gpioc, 13, 0);
    break;

  default:
    return -1;
    break;
  }

  return 0;
}

/* USER CODE END 2 */
