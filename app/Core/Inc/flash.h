#ifndef __FLASH_H__
#define __FLASH_H__

#include <stdint.h>

#define FLASH_PAGE_SIZE 1024

int flash_init(void);

int flash_erase_page(uint32_t page_addr);

int flash_write(uint32_t addr, uint8_t *data, uint32_t len);

int flash_read(uint32_t addr, uint8_t *data, uint32_t len);

#endif //__FALSH_H__