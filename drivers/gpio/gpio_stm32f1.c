#include "device.h"
#include "drivers/gpio.h"
#include <stdint.h>
#include "stm32f103x6.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_bus.h"

int gpio_stm32_config(const device_t *port,
                      gpio_pin_t pin,
                      gpio_flags_t flags)
{
    LL_GPIO_InitTypeDef led_init;

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOC);

    led_init.Pin = LL_GPIO_PIN_13;
    led_init.Mode = LL_GPIO_MODE_OUTPUT;
    led_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    led_init.Speed = LL_GPIO_SPEED_FREQ_LOW;
    LL_GPIO_Init(GPIOC, &led_init);

    LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_13);

    return 0;
}

int gpio_stm32_set_bit_raw(const device_t *port,
                           gpio_port_pins_t pins)
{
    return 0;
}

int gpio_stm32_clear_bit_raw(const device_t *port,
                             gpio_port_pins_t pins)
{
    return 0;
}

int gpio_stm32_init()
{
    LL_GPIO_InitTypeDef led_init;

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOC);

    led_init.Pin = LL_GPIO_PIN_13;
    led_init.Mode = LL_GPIO_MODE_OUTPUT;
    led_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    led_init.Speed = LL_GPIO_SPEED_FREQ_LOW;
    LL_GPIO_Init(GPIOC, &led_init);

    LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_13);

    return 0;
}

gpio_driver_api_t gpio_stm32_api = {
    .pin_config = &gpio_stm32_config,
    .port_set_bit_raw = &gpio_stm32_set_bit_raw,
    .port_clear_bit_raw = &gpio_stm32_clear_bit_raw,
};

struct gpio_driver_data {
    int a;
};

struct gpio_driver_config {
    int a;
};

struct gpio_driver_data stm32_data;
struct gpio_driver_config stm32_config;
device_state_t stm32_state;

DEVICE_DEFINE(gpioc, "gpioc",     \
              &gpio_stm32_init,        \
              NULL,                \
              &stm32_data,         \
              &stm32_config,        \
              1,                \
              1,                \
              &gpio_stm32_api,       \
              &stm32_state);
