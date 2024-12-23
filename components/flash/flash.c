#include "flash.h"
#include "stm32f1xx.h"
#include <string.h>

static inline void flash_wait_for_last_operation(void)
{
    while((FLASH->SR & FLASH_SR_BSY) != 0);  // 等待上一次操作完成
}

int flash_init(void)
{
    if((FLASH->CR & FLASH_CR_LOCK) != 0) {
        FLASH->KEYR = FLASH_KEY1;  // Key1
        FLASH->KEYR = FLASH_KEY2;  // Key2
    }

    return 0;
}

int flash_erase_page(uint32_t page_addr)
{
    flash_wait_for_last_operation();

    // 设置页擦除
    FLASH->CR |= FLASH_CR_PER;
    FLASH->AR = page_addr;
    FLASH->CR |= FLASH_CR_STRT;
    
    flash_wait_for_last_operation();
    
    // 清除页擦除标志
    FLASH->CR &= ~FLASH_CR_PER;

    return 0;
}

int flash_write(uint32_t addr, uint8_t *data, uint32_t len)
{
    uint32_t i;
    
    if (addr % 2) return -1;  // 确保地址对齐
    
    flash_wait_for_last_operation();
    
    // 设置编程模式
    FLASH->CR |= FLASH_CR_PG;
    
    // 按半字写入
    for (i = 0; i < len; i += 2) {
        *(volatile uint16_t*)(addr + i) = *(uint16_t*)(data + i);
        flash_wait_for_last_operation();
    }
    
    // 清除编程标志
    FLASH->CR &= ~FLASH_CR_PG;

    return 0;
}

int flash_read(uint32_t addr, uint8_t *data, uint32_t len)
{
    memcpy(data, (void*)addr, len);
    return 0;
}