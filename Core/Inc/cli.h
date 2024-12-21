#ifndef __CLI_H__
#define __CLI_H__

typedef int (*cli_cmd_handler_t)(int argc, char *argv[]);

typedef struct {
    const char *cmd;
    const char *description;
    cli_cmd_handler_t handler;
} cli_command_t;

/**
 * @brief 初始化函数
 * 
 * @return int 
 */
int cli_init();

/**
 * @brief 指令注册接口
 * 
 * @param command 
 * @return int 
 */
int cli_register_command(const cli_command_t *command);

/**
 * @brief 指令处理
 * 
 * @param ch 
 * @return int 
 */
int cli_process_char(char ch);

#endif //__CLI_H__