#include "boot.h"
#include "flash.h"
#include "log.h"
#include <stdint.h>
#include <string.h>
#include "stm32f1xx.h"

typedef void (*pFunction)(void);  // 定义函数指针类型，指向一个无参数无返回值的函数

static firmware_state_t fw_state;

int boot_init(void)
{
    if (flash_init() != 0) {
        LOG_ERR("Boot init failed!");
        return -1;
    }

    memset(&fw_state, 0, sizeof(fw_state));
    LOG_INFO("Bootloader Version: %s", BOOT_VERSION);
    return 0;
}

int boot_start_update(firmware_header_t *header)
{
    if (header->magic != BOOT_MAGIC_NUM) {
        LOG_ERR("Invalid firmware magic");
        return -1;
    }

    if (header->size > APP_SIZE_MAX) {
        LOG_ERR("Firmware too large");
        return -1;
    }

    memcpy(&fw_state.header, header, sizeof(firmware_header_t));
    fw_state.received_size = 0;
    fw_state.current_addr = APP_START_ADDR;
    fw_state.status = UPDATE_READY;

    LOG_INFO("Ready to receive firmware v%d, size: %d bytes",
            header->version, header->size);

    return 0;
}

int boot_receive_date(uint8_t *data, uint32_t size)
{
    if (fw_state.status != UPDATE_READY &&
        fw_state.status != UPDATE_IN_PROGRESS) {
        return -1;
    }

    if (fw_state.status == UPDATE_READY) {
        fw_state.status = UPDATE_IN_PROGRESS;
    }

    if ((fw_state.received_size + size) > fw_state.header.size) {
        LOG_ERR("Received data exceeds firmware size");
        fw_state.status = UPDATE_ERROR;
        return -1;
    }

    if (boot_update_app(data, size) != 0) {
        LOG_ERR("Failed to write firmare data");
        fw_state.status = UPDATE_ERROR;
        return -1;
    }

    fw_state.received_size += size;

    if (fw_state.received_size == fw_state.header.size) {
        fw_state.status = UPDATE_COMPLETE;
        LOG_INFO("Firmware update complete");
    }

    return 0;
}

int boot_check_app(void)
{
    uint32_t *app_addr = (uint32_t *)APP_START_ADDR;

    if ((app_addr[0] & 0x2FFE0000) != 0x20000000) {
        LOG_ERR("Invalid stack address");
        return -1;
    }

    if ((app_addr[1] & 0x2FFE0001) != 0x08000001) {
        LOG_ERR("Invalid reset vector");
        return -1;
    }

    return 0;
}

void boot_jump_to_app(void)
{
    uint32_t jump_addr;
    pFunction jump_to_app;

    jump_addr = *((volatile uint32_t *)(APP_START_ADDR + 4));
    jump_to_app = (pFunction)jump_addr;

    __disable_irq();

    SCB->VTOR = APP_START_ADDR;

    __set_MSP(*((volatile uint32_t *)APP_START_ADDR));

    jump_to_app();
}

int boot_update_app(uint8_t *data, uint32_t size)
{
    uint32_t remaining = size;
    uint32_t current_page = 0;

    if (size > APP_SIZE_MAX) {
        LOG_ERR("App size too large");
        return -1;
    }

    if (fw_state.received_size == 0) {
        for (current_page = APP_START_ADDR;
            current_page < (APP_START_ADDR + size);
            current_page += FLASH_PAGE_SIZE) {
            if(flash_erase_page(current_page) != 0) {
                LOG_ERR("Erase failed at 0x%08X", current_page);
                return -1;
            }
        }
    }

    while (remaining > 0) {
        uint32_t write_size = (remaining > 256) ? 256 : remaining;

        if (flash_write(fw_state.current_addr, data, write_size) != 0) {
            LOG_ERR("Write failed at 0x%08X", fw_state.current_addr);
            return -1;
        }

        fw_state.current_addr += write_size;
        data += write_size;
        remaining -= write_size;
    }

    return 0;
}
