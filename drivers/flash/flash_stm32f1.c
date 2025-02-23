#include "device.h"
#include "drivers/flash.h"
#include "flash_stm32f1.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static inline void flash_wait_for_last_operation(const struct device *dev)
{
	const struct flash_stm32_config *config = 
			(struct flash_stm32_config *)dev->config;

    while((config->base->SR & FLASH_SR_BSY) != 0);  // 等待上一次操作完成
}

static int stm32_flash_read(const struct device *dev,
				off_t offset, void *data, size_t len)
{
	memcpy(data, (void*)offset, len);
	return 0;
}

static int stm32_flash_write(const struct device *dev,
				off_t offset, const void *data, size_t len)
{
	const struct flash_stm32_config *config = 
			(struct flash_stm32_config *)dev->config;
	uint32_t i;
    
    if (offset % 2) return -1;  // 确保地址对齐
    
    flash_wait_for_last_operation(dev);
    
    // 设置编程模式
    config->base->CR |= FLASH_CR_PG;
    
    // 按半字写入
    for (i = 0; i < len; i += 2) {
        *(volatile uint16_t *)(offset + i) = *(uint16_t *)(data + i);
        flash_wait_for_last_operation(dev);
    }
    
    // 清除编程标志
    config->base->CR &= ~FLASH_CR_PG;

    return 0;
}

static int stm32_flash_erase(const struct device *dev,
				off_t offset, size_t len)
{
	const struct flash_stm32_config *config = 
			(struct flash_stm32_config *)dev->config;

	flash_wait_for_last_operation(dev);

    // 设置页擦除
    config->base->CR |= FLASH_CR_PER;
    config->base->AR = offset;
    config->base->CR |= FLASH_CR_STRT;
    
    flash_wait_for_last_operation(dev);
    
    // 清除页擦除标志
    config->base->CR &= ~FLASH_CR_PER;

	return 0;
}

static const struct flash_parameters *stm32_flash_get_param(	
							const struct device *dev)
{
    const struct flash_stm32_config *config = 
            (struct flash_stm32_config *)dev->config;

    if((config->base->CR & FLASH_CR_LOCK) != 0) {
        config->base->KEYR = FLASH_KEY1;  // Key1
        config->base->KEYR = FLASH_KEY2;  // Key2
    }

    return 0;
}

static int stm32_flash_init(const struct device *dev)
{
	// fix:时钟问题，应用层变更时钟后会影响外设，
	//     先临时用上面的在应用层初始化。
    // const struct flash_stm32_config *config = 
    //         (struct flash_stm32_config *)dev->config;

    // if((config->base->CR & FLASH_CR_LOCK) != 0) {
    //     config->base->KEYR = FLASH_KEY1;  // Key1
    //     config->base->KEYR = FLASH_KEY2;  // Key2
    // }

    return 0;
}

struct flash_driver_api flash_stm32_api = {
	.read = stm32_flash_read,
    .write = stm32_flash_write,
	.erase = stm32_flash_erase,
	.get_parameters = stm32_flash_get_param,
};

struct flash_stm32_config stm32_config= {
    .base = FLASH
};

struct flash_stm32_data stm32_data = {
    .res = NULL
};

static device_state_t flash_stm32_state;

DEVICE_DEFINE(                  \
    flash, "flash",                  \
    &stm32_flash_init,                  \
    NULL,                           \
    &stm32_data,            \
    &stm32_config,          \
    1, 1,                                       \
    &flash_stm32_api,                \
    &flash_stm32_state             \
);
