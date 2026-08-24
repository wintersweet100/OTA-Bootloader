/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-04-30     Bernard      the first version for FinSH
 * 2006-05-08     Bernard      change finsh thread stack to 2048
 * 2006-06-03     Bernard      add support for skyeye
 * 2006-09-24     Bernard      remove the code related with hardware
 * 2010-01-18     Bernard      fix down then up key bug.
 * 2010-03-19     Bernard      fix backspace issue and fix device read in shell.
 * 2010-04-01     Bernard      add prompt output when start and remove the empty history
 * 2011-02-23     Bernard      fix variable section end issue of finsh shell
 *                             initialization when use GNU GCC compiler.
 * 2016-11-26     armink       add password authentication
 * 2018-07-02     aozima       add custom prompt support.
 */

#include "command.h"
#include "usart.h"
#include "string.h"
#include "flash_cmd.h" // 确保能访问 get_flash 和 flash_ops [2]

extern void start_app(unsigned int vector); 
extern TIM_HandleTypeDef htim6;
extern struct flash_ops *get_flash(void); // 获取 flash 操作接口 [2]

static int go_f(int argc, char **argv)
{
    unsigned int addr = 0x08009000; // 默认跳转地址 [3]0x20000000
    struct flash_ops *fp = get_flash();
    unsigned int check_buf[4]; // 用于读取起始地址的几个字节进行判断
    int is_invalid = 1;

    // 1. 处理参数：如果输入了第二个参数，则使用输入的地址 [2][4]
    if (argc == 2)
    {
        addr = str2hex(argv[1]);
    }

    // 2. 跳转前判断：使用 flash.read 读取起始地址数据 [2]
    // 假设读取前 16 字节进行校验
    if (fp && fp->read)
    {
        if (fp->read((unsigned char *)check_buf, addr, sizeof(check_buf)) == sizeof(check_buf))
        {
            for (int i = 0; i < 4; i++)
            {
                // 如果发现任何一个字节不是 0x00 且不是 0xFF，则认为可能存在有效代码
                if (check_buf[i] != 0x00000000 && check_buf[i] != 0xFFFFFFFF)
                {
                    is_invalid = 0;
                    break;
                }
            }
        }
    }

    if (is_invalid)
    {
        printf("跳转地址处无正确代码\r\n");
        return -1;
    }

    // 3. 准备跳转：清理硬件环境 [3]
    printf("Jumping to address: 0x%08X\r\n", addr);
    
    __disable_irq();
    HAL_TIM_Base_Stop_IT(&htim6); // 关闭定时器 [3]
    
    // 关闭 SysTick、清除挂起的中断
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
    for (uint32_t i = 0; i < 8; i++)
    {
        NVIC->ICER[i] = 0xFFFFFFFF; // 禁用所有中断 [3]
        NVIC->ICPR[i] = 0xFFFFFFFF; // 清除挂起标志 [3]
    }

    // 执行跳转
    start_app(addr);
    
    return 0; // 正常情况下不会执行到这里 [3]
}

struct command go_cmd = {
    "go",
    "jump to specific address (default 0x08009000) to run app\r\n",
    "Usage: \r\n"
    "       go [addr]\r\n", // 更新用法说明 [4]
    go_f,
};



//#include "command.h"
//#include "usart.h"

//extern void start_app(unsigned int vector); 
//extern TIM_HandleTypeDef htim6;

//static int go_f(int argc, char **argv)
//{
//#if 0	
//	typedef void (*f_type)(void);
//	f_type f;
//	f = (f_type)0x20000000;
//	f();
//#else
//	__disable_irq();
//	HAL_TIM_Base_Stop_IT(&htim6); // 先关掉你开启的定时器
//	// 关闭 SysTick、清除挂起的中断（干净的执行环境）
//	SysTick->CTRL = 0;
//	SysTick->LOAD = 0;
//	SysTick->VAL = 0;
//	for (uint32_t i = 0; i < 8; i++)
//	{
//		NVIC->ICER[i] = 0xFFFFFFFF; // 禁用所有中断
//		NVIC->ICPR[i] = 0xFFFFFFFF; // 清除挂起标志
//	}
//	start_app(0x20000000);
//#endif
//	
//	return 0;//根本到不了这里
//}

//struct command go_cmd = {
//	"go",
//		
//	"jump to 0x20000000 to rum app\r\n",
//	
//	"Usage: \r\n"
//	"       go, jump to 0x20000000 to rum app\r\n",

//	go_f,
//};















