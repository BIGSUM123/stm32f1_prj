#ifndef __GPIO_STM32F1_H__
#define __GPIO_STM32F1_H__

#include <stdint.h>
#include "drivers/gpio.h"

// struct stm32_pclken {
// 	uint32_t bus : STM32_CLOCK_DIV_SHIFT;
// 	uint32_t div : (32 - STM32_CLOCK_DIV_SHIFT);
// 	uint32_t enr;
// };

/**
 * @brief configuration of GPIO device
 */
struct gpio_stm32_config {
	/* gpio_driver_config needs to be first */
	struct gpio_driver_config common;
	/* port base address */
	uint32_t *base;
	/* IO port */
	int port;
	// struct stm32_pclken pclken;
};

/**
 * @brief driver data
 */
struct gpio_stm32_data {
	/* gpio_driver_data needs to be first */
	struct gpio_driver_data common;
	/* device's owner of this data */
	const struct device *dev;
	// /* user ISR cb */
	// sys_slist_t cb;
	/* keep track of pins that  are connected and need GPIO clock to be enabled */
	uint32_t pin_has_clock_enabled;
};

#endif // __GPIO_STM32F1_H__