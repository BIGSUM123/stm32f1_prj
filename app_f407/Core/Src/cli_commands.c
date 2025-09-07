#include "cli_commands.h"
#include "cli.h"
#include "kernel.h"
#include "log.h"
#include "gpio.h"
#include <string.h>
#include "stm32f4xx.h"

static int cmd_help(int argc, char *argv[])
{
    log_printf("\r\nAvailable commands:\r\n");
    log_printf("  help       - Show this help message\r\n");
    log_printf("  led        - Control LED (on/off)\r\n");
    log_printf("  version    - Show firmware version\r\n");
    log_printf("  switch     - Trigger context switch\r\n");
    log_printf("  thread     - Show current thread info\r\n");
    log_printf("  sem_test   - Test semaphore operations\r\n");
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

static int cmd_switch(int argc, char *argv[])
{
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
    __DSB();
    __ISB();

    return 0;
}

static int cmd_thread(int argc, char *argv[])
{
    extern tcb_t *pxCurrentTCB;
    log_printf("%p\r\n", pxCurrentTCB);
    return 0;
}

static int cmd_sem_test(int argc, char *argv[])
{
    extern semaphore_handle_t binary_sem, counting_sem;
    
    if (argc < 2) {
        log_printf("Usage: sem_test <binary|counting> [post|wait]\r\n");
        log_printf("  sem_test binary post   - Post to binary semaphore\r\n");
        log_printf("  sem_test binary wait   - Wait on binary semaphore\r\n");
        log_printf("  sem_test counting post - Post to counting semaphore\r\n");
        log_printf("  sem_test counting wait - Wait on counting semaphore\r\n");
        log_printf("  sem_test status        - Show semaphore status\r\n");
        return -1;
    }

    if (strcmp(argv[1], "status") == 0) {
        log_printf("Binary semaphore count: %lu\r\n", sem_get_count(binary_sem));
        log_printf("Counting semaphore count: %lu\r\n", sem_get_count(counting_sem));
        return 0;
    }

    if (argc != 3) {
        log_printf("Usage: sem_test <binary|counting> [post|wait]\r\n");
        return -1;
    }

    semaphore_handle_t sem = NULL;
    if (strcmp(argv[1], "binary") == 0) {
        sem = binary_sem;
    } else if (strcmp(argv[1], "counting") == 0) {
        sem = counting_sem;
    } else {
        log_printf("Invalid semaphore type. Use 'binary' or 'counting'\r\n");
        return -1;
    }

    if (strcmp(argv[2], "post") == 0) {
        rtos_error_t result = sem_post(sem);
        if (result == RTOS_OK) {
            log_printf("Semaphore posted successfully (count=%lu)\r\n", sem_get_count(sem));
        } else {
            log_printf("Failed to post semaphore (error=%d)\r\n", result);
        }
    } else if (strcmp(argv[2], "wait") == 0) {
        log_printf("Waiting on semaphore (timeout=2s)...\r\n");
        rtos_error_t result = sem_wait(sem, rtos_ms_to_ticks(2000));
        if (result == RTOS_OK) {
            log_printf("Semaphore acquired successfully (count=%lu)\r\n", sem_get_count(sem));
        } else if (result == RTOS_TIMEOUT) {
            log_printf("Semaphore wait timeout\r\n");
        } else {
            log_printf("Failed to wait on semaphore (error=%d)\r\n", result);
        }
    } else {
        log_printf("Invalid operation. Use 'post' or 'wait'\r\n");
        return -1;
    }

    return 0;
}

const static cli_command_t basic_commands[] = {
    {"help", "Show available commands", cmd_help},
    {"led", "Contrl led (on/off)", cmd_led},
    {"version", "Show firmware version", cmd_version},
    {"switch", "Change thread", cmd_switch},
    {"thread", "get thread type", cmd_thread},
    {"sem_test", "Test semaphore operations", cmd_sem_test}
};

int cli_register_basic_commands()
{
    for (int i = 0; i < sizeof(basic_commands)/sizeof(basic_commands[0]); i++) {
        cli_register_command(&basic_commands[i]);
    }

    return 0;
}
