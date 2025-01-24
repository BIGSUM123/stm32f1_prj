#include "device.h"
#include "drivers/gpio.h"
#include <stdint.h>
#include "stm32f103x6.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_bus.h"
#include "gpio_stm32f1.h"

static int gpio_stm32_config(const struct device *port,
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

static int gpio_stm32_set_bit_raw(const struct device *port,
                           gpio_port_pins_t pins)
{
    LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_13);
    return 0;
}

static int gpio_stm32_clear_bit_raw(const struct device *port,
                             gpio_port_pins_t pins)
{
    LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_13);
    return 0;
}

int gpio_stm32_init(const struct device *dev)
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

#define GPIO_DEVICE_DEFINE(__suffix, __SUFFIX)        \
    static const struct gpio_stm32_config gpio_stm32_cfg_##__suffix = { \
        .common = {                 \
            .port_pin_mask = BIT(13),                   \
        },                                          \
        .base = (uint32_t *)__SUFFIX,              \
        .port = 1,                          \
    };                               \
    static struct gpio_stm32_data gpio_stm32_data_##__suffix;       \
    static device_state_t gpio_stm32_state_##__suffix;          \
    DEVICE_DEFINE(                                  \
        __suffix, #__suffix,                  \
        &gpio_stm32_init,                \
        NULL,                               \
        &gpio_stm32_data_##__suffix,            \
        &gpio_stm32_cfg_##__suffix,          \
        1, 1,                                       \
        &gpio_stm32_api,                \
        &gpio_stm32_state_##__suffix             \
    )

#define GPIO_DEVICE_DEFINE_STM32(__suffix, __SUFFIX)        \
    GPIO_DEVICE_DEFINE(gpio##__suffix, GPIO##__SUFFIX)

GPIO_DEVICE_DEFINE_STM32(c, C);
