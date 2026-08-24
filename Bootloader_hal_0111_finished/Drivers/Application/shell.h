/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-06-02     Bernard      Add finsh_get_prompt function declaration
 */

#ifndef __SHELL_H__
#define __SHELL_H__

#include "types.h"
#include <stdio.h>

#ifdef putchar
  #undef putchar
#endif

#ifdef getchar
  #undef getchar
#endif

#define putchar  shell_putchar
#define getchar  shell_getchar

#ifndef FINSH_CMD_SIZE
#define FINSH_CMD_SIZE      80
#endif

#define FINSH_OPTION_ECHO   0x01

#define FINSH_PROMPT        finsh_get_prompt()
const char* finsh_get_prompt(void);
//int finsh_set_prompt(const char * prompt);

#define FINSH_HISTORY_LINES 15  //缓存的历史命令总数


enum input_stat
{
    WAIT_NORMAL,
    WAIT_SPEC_KEY,
    WAIT_FUNC_KEY,
};
struct finsh_shell
{

    enum input_stat stat;

    rt_uint8_t echo_mode:1;
    rt_uint8_t prompt_mode: 1;

    rt_uint16_t current_history;
    rt_uint16_t history_count;

    char cmd_history[FINSH_HISTORY_LINES][FINSH_CMD_SIZE];


    char line[FINSH_CMD_SIZE + 1];
    rt_uint16_t line_position;
    rt_uint16_t line_curpos;
};

void finsh_set_echo(rt_uint32_t echo);
rt_uint32_t finsh_get_echo(void);

//int finsh_system_init(void);
//void finsh_set_device(const char* device_name);
//const char* finsh_get_device(void);

rt_uint32_t finsh_get_prompt_mode(void);
void finsh_set_prompt_mode(rt_uint32_t prompt_mode);
void shell(void);

#endif

