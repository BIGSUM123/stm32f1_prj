#ifndef __LOG_H__
#define __LOG_H__

typedef enum {
    LOG_LEVEL_NONE = 0,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_MAX,
} log_level_t;

/**
 * @brief 
 * 
 */
void log_init();

/**
 * @brief 
 * 
 * @param level 
 * @param file 
 * @param line 
 * @param format 
 * @param ... 
 */
void log_write(log_level_t level, const char *file, int line, const char *format, ...);

/**
 * @brief 
 * 
 * @param format 
 * @param ... 
 */
void log_printf(const char *format, ...);

#if LOG_LEVEL >= LOG_LEVEL_ERROR
    #define LOG_ERR(fmt, ...) log_write(LOG_LEVEL_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#else
    #define LOG_ERR(fmt, ...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_WARN
    #define LOG_WARN(fmt, ...) log_write(LOG_LEVEL_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#else
    #define LOG_WARN(fmt, ...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_INFO
    #define LOG_INFO(fmt, ...) log_write(LOG_LEVEL_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#else
    #define LOG_INFO(fmt, ...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_DEBUG
    #define LOG_DBG(fmt, ...) log_write(LOG_LEVEL_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#else
    #define LOG_DBG(fmt, ...)
#endif

#endif // __LOG_H__