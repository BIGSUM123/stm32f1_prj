#ifndef __LOG_PORT_H__
#define __LOG_PORT_H__

#include <stdint.h>

/**
 * @brief 
 * 
 */
int log_port_init();

/**
 * @brief 
 * 
 * @param date 
 * @param len 
 */
void log_port_write(const char *date, uint16_t len);

/**
 * @brief 接收函数
 * 
 * @param data 
 * @return int 
 */
int log_port_receive(uint8_t *data);

#endif // __LOG_PORT_H__