#ifndef __FLASH_STM32F1_H__
#define __FLASH_STM32F1_H__

#include "stm32f103x6.h"

#ifdef __cplusplus
extern "C" {
#endif

struct flash_stm32_config {
    FLASH_TypeDef *base;
};

struct flash_stm32_data {
    void *res;
};

#ifdef __cplusplus
}
#endif

#endif // __FLASH_STM32F1_H__