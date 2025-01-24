#include "log.h"
#include "log_config.h"
#include "log_port.h"
#include <stdio.h>
#include <stdarg.h>

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
void log_init()
{
    log_port_init();
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

    log_port_write(buffer, len);
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

    log_port_write(buffer, len);
}
