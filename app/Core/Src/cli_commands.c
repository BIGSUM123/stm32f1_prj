#include "cli_commands.h"
#include "cli.h"
#include "log.h"
#include "gpio.h"
#include <stdint.h>
#include <string.h>
#include "flash.h"

static int cmd_help(int argc, char *argv[])
{
    log_printf("\r\nAvailable commands:\r\n");
    log_printf("  help       - Show this help message\r\n");
    log_printf("  led        - Control LED (on/off)\r\n");
    log_printf("  version    - Show firmware version\r\n");
    return 0;
}

static int cmd_led(int argc, char *argv[])
{
    if (argc != 2) {
        log_printf("Usage: led <on/off>\r\n");
        return -1;
    }

    if (strcmp(argv[1], "on") == 0) {
        led_ctrl(LED_ON);
    }
    else if (strcmp(argv[1], "off") == 0) {
        led_ctrl(LED_OFF);
    }
    else {
        log_printf("Invalid parameter. Use 'on' or 'off'\r\n");
        return -1;
    }

    return 0;
}

static int cmd_version(int argc, char *argv[])
{
    log_printf("Firmare Version: 1.0.0\r\n");
    return 0;
}

static int cmd_flash_test(int argc, char *argv[])
{
    uint8_t test_data[] = "Hello Flash!";
    uint8_t read_data[20];
    uint32_t test_addr = 0x08004000;

    if (flash_init() != 0) {
        log_printf("Flash init failed!\r\n");
        return -1;
    }

    if (flash_write(test_addr, test_data, strlen((char *)test_data)) != 0) {
        log_printf("Flash write failed!");
        return -1;
    }

    if (flash_read(test_addr, read_data, strlen((char *)test_data)) != 0) {
        log_printf("Flash read failed!\r\n");
        return -1;        
    }

    read_data[strlen((char *)test_data)] = 0;
    log_printf("Read from flash: %s\r\n", read_data);

    if (flash_erase_page(test_addr) != 0) {
        log_printf("Flash erase failed!\r\n");
        return -1;
    }

    memset(read_data, 0, 20);
    if (flash_read(test_addr, read_data, strlen((char *)test_data)) != 0) {
        log_printf("Flash read failed!\r\n");
        return -1;        
    }
    read_data[strlen((char *)test_data)] = 0;
    log_printf("Read after erase: %s\r\n", read_data);

    return 0;
}

const static cli_command_t basic_commands[] = {
    {"help", "Show available commands", cmd_help},
    {"led", "Contrl led (on/off)", cmd_led},
    {"version", "Show firmware version", cmd_version},
    {"flash_test", "Test flash operations", cmd_flash_test}
};

int cli_register_basic_commands()
{
    for (int i = 0; i < sizeof(basic_commands)/sizeof(basic_commands[0]); i++) {
        cli_register_command(&basic_commands[i]);
    }

    return 0;
}
