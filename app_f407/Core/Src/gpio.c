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
const struct device *gpiob;
/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void led_init(void)
{
  gpiob = device_get_binding("gpiob");
  gpio_pin_configure(gpiob, 2, 0);
}

/* USER CODE BEGIN 2 */

int led_ctrl(led_state state)
{
  switch (state) {
  case LED_OFF:
    // LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_13);
    gpio_pin_set_raw(gpiob, 2, 1);
    break;

  case LED_ON:
    // LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_13);
    gpio_pin_set_raw(gpiob, 2, 0);
    break;

  default:
    return -1;
    break;
  }

  return 0;
}

/* USER CODE END 2 */
