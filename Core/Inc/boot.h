#ifndef __BOOT_H__
#define __BOOT_H__

#include <stdint.h>

#define BOOT_VERSION    "1.0.0"
#define APP_START_ADDR  0x08004000
#define APP_SIZE_MAX    (64 * 1024)
#define BOOT_MAGIC_NUM  0x55AA55AA

#pragma pack(1)
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t size;
    uint32_t crc;
} firmware_header_t;
#pragma pack()

typedef enum {
    UPDATE_IDLE = 0,
    UPDATE_READY,
    UPDATE_IN_PROGRESS,
    UPDATE_COMPLETE,
    UPDATE_ERROR
} update_status_t;

typedef struct {
    update_status_t status;
    firmware_header_t header;
    uint32_t received_size;
    uint32_t current_addr;
} firmware_state_t;

int boot_init(void);

int boot_check_app(void);

void boot_jump_to_app(void);

int boot_update_app(uint8_t *data, uint32_t size);

#endif // __BOOT_H__