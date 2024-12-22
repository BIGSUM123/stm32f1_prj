#include "cli.h"
#include "cli.h "
#include "log.h"
#include <stdint.h>
#include <string.h>
#include <stddef.h>

#define CLI_MAX_COMMANDS    20
#define CLI_BUF_SIZE        128
#define CLI_MAX_ARGC        10
#define CLI_PROMPT          "cli>"

static cli_command_t commands[CLI_MAX_COMMANDS];
static uint8_t command_count = 0;

static char cli_buf[CLI_BUF_SIZE];
static uint16_t cli_buf_index = 0;

static void cli_process_command(char *cmd_line);

int cli_init()
{
    command_count = 0;
    cli_buf_index = 0;
    LOG_INFO("CLI system initialized");
    log_printf("\r\nCLI>");
    return 0;
}

int cli_register_command(const cli_command_t *command)
{
    if (command_count >= CLI_MAX_COMMANDS) {
        return -1;
    }

    commands[command_count++] = *command;
    return 0;
}

int cli_process_char(char ch)
{
    if (ch == '\r' || ch == '\n') {
        log_printf("\r\n");
        if (cli_buf_index > 0) {
            cli_buf[cli_buf_index] = '\0';
            cli_process_command(cli_buf);
            cli_buf_index = 0;
        }
    }
    else if (cli_buf_index < (CLI_BUF_SIZE - 1)) {
        cli_buf[cli_buf_index++] = ch;
        log_printf("%c", ch);
    }
    return 0;
}

static void cli_process_command(char *cmd_line)
{
    char *argv[CLI_MAX_ARGC];
    int argc = 0;
    char *token;

    token = strtok(cmd_line, " ");
    while (token != NULL && argc < CLI_MAX_ARGC) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }

    if (argc == 0) return;

    for (int i = 0; i < CLI_MAX_COMMANDS; i++) {
        if (strcmp(argv[0], commands[i].cmd)  == 0) {
            commands[i].handler(argc, argv);
            return;
        }
    }

    log_printf("Unknown command: %s\r\n", argv[0]);
}
