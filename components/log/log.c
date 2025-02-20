#include "log.h"
#include "log_config.h"
#include <stdio.h>
#include <stdarg.h>
#include "drivers/uart.h"

/**
 * @brief 
 * 
 */
static const char *level_strings[] = {
    "NONE",
    "ERR",
    "WARN",
    "INFO",
    "DBG",
};

/**
 * @brief 
 * 
 */
const struct device *log_uart;

/**
 * @brief 
 * 
 */
void log_init()
{
    log_uart = device_get_binding("uart1");
}

/**
 * @brief 
 * 
 * @param level 
 * @param format 
 * @param ... 
 */
void log_write(log_level_t level, const char *file, int line, const char *format, ...)
{
    char buffer[LOG_BUFFER_SIZE];
    char *ptr = buffer;
    int len = 0;
    va_list args;

    len += snprintf(ptr + len, LOG_BUFFER_SIZE - len, "[%s]>> ", level_strings[level]);

#if LOG_SHOW_FILE_LINE
    len += snprintf(ptr + len, LOG_BUFFER_SIZE - len, "%s:%d ", file, line);
#endif

    va_start(args, format);
    len += vsnprintf(ptr + len, LOG_BUFFER_SIZE - len, format, args);
    va_end(args);

    len += snprintf(ptr + len, LOG_BUFFER_SIZE - len, "\r\n");

    for (int i = 0; i < len; i++) {
        uart_poll_out(log_uart, buffer[i]);
    }
}

/**
 * @brief 
 * 
 */
int log_read(uint8_t *data)
{
    if (data == NULL) {
        return -1;
    }

    if (uart_poll_in(log_uart, data) == 0) {
        return 1;
    }

    return 0;
}

/**
 * @brief 
 * 
 * @param format 
 * @param ... 
 */
void log_printf(const char *format, ...)
{
    char buffer[LOG_BUFFER_SIZE];
    va_list args;
    int len = 0;

    va_start(args, format);
    len += vsnprintf(buffer, LOG_BUFFER_SIZE, format, args);
    va_end(args);

    for (int i = 0; i < len; i++) {
        uart_poll_out(log_uart, buffer[i]);
    }
}
