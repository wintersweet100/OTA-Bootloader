/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
#include "ring_buffer.h"
#include "socket.h"
#include "w5500.h"
#include "bsp_esp8266.h"

#define DEBUG_UART_TIMEOUT 500

static UART_HandleTypeDef * g_HDebugUART = &huart1;
ring_buffer rx_buf;
extern UART_HandleTypeDef huart_esp8266;

uint8_t g_WiFi_Shell_Enable = 0; //用于控制usart3是否转发数据，转发的话会发送到esp8266端进行回显，默认关闭转发

/* USER CODE END 0 */

UART_HandleTypeDef huart1;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */
    USART_TypeDef *usart1 = USART1;  // 使用HAL定义的宏
    
    // 使能接收中断
    usart1->CR1 |= (1<<5);  // RXNEIE
    
    // 初始化环形缓冲区
    ring_buffer_init(&rx_buf);
  /* USER CODE END USART1_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
extern UART_HandleTypeDef huart_esp8266;
// 定义ESP发送缓存
#define ESP_LINE_BUF_SIZE  128
static uint8_t g_esp_buf[ESP_LINE_BUF_SIZE];
static uint16_t g_esp_ptr = 0;

#define W5500_LINE_BUF_SIZE  128
static uint8_t g_w5500_buf[W5500_LINE_BUF_SIZE];
static uint16_t g_w5500_ptr = 0;

/**
 * @brief ESP8266发送缓存数据到客户端
 */
void esp_flush_buffer(void)
{
    if(g_esp_ptr > 0 && g_WiFi_Shell_Enable)
    {
        // AP模式Server需要用AT+CIPSEND指令发送
        // 假设发送给第一个客户端 ID=0
        if(!ESP8266_SendString(DISABLE, (char*)g_esp_buf, g_esp_ptr, Multiple_ID_0))
		{ 
			printf("[ESP8266] Send failed, retrying...\r\n");
			if(!ESP8266_SendString(DISABLE, (char*)g_esp_buf, g_esp_ptr, Multiple_ID_0))
			{
				g_WiFi_Shell_Enable = 0;
				printf("\r\nSend failed,client may disconnected\r\n");			
			}
		}
        g_esp_ptr = 0;
    }
}

/**
* @brief 真正的 W5500 发送执行函数
*/
void w5500_flush_buffer(void)
{
    if(g_w5500_ptr > 0 && getSn_SR(SOCK_TCPS) == SOCK_ESTABLISHED)
    {
        send(SOCK_TCPS, g_w5500_buf, g_w5500_ptr);
        g_w5500_ptr = 0; // 发送完清空指针
    }
}


int fputc(int c, FILE *f) 
{
    (void)f;
	HAL_UART_Transmit(g_HDebugUART, (const uint8_t *)&c, 1, DEBUG_UART_TIMEOUT);
	
   // 2.只有在 ESP8266 初始化完成后才发送，避免初始化阶段乱码
//	if(g_WiFi_Shell_Enable)
//	{
//	    //HAL_UART_Transmit(&huart_esp8266, (const uint8_t *)&c, 1, DEBUG_UART_TIMEOUT);
//        // 缓存数据
//        if(g_esp_ptr < ESP_LINE_BUF_SIZE)
//        {
//            g_esp_buf[g_esp_ptr++] = (uint8_t)c;
//        }
//        
//        // 遇到换行或缓冲区满时发送
//        if(c == '\n' || g_esp_ptr >= ESP_LINE_BUF_SIZE-1)
//        {
//            esp_flush_buffer();
//        }	
//	}

    // 3. W5500-判断是否需要立即发送（刷新）
	if(getSn_SR(SOCK_TCPS) == SOCK_ESTABLISHED)
	{
		HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
		send(SOCK_TCPS,(const uint8_t *)&c,1);
		HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
	}

//    if(g_w5500_ptr < W5500_LINE_BUF_SIZE)
//    {
//        g_w5500_buf[g_w5500_ptr++] = c;
//    }
// //条件：遇到换行符，或者是 Shell 的提示符 '>'，或者是缓冲区满了
//    if(c == '\n' || c == '\r'  || g_w5500_ptr >= W5500_LINE_BUF_SIZE)
//    {
//		HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
//		w5500_flush_buffer();
//		HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
//    }
    return c;
}

int fgetc(FILE *f)
{
    //uint8_t ch = 0;
    (void)f;

	return getchar2();
/*
    // Clear the Overrun flag just before receiving the first character 
    __HAL_UART_CLEAR_OREFLAG(g_HDebugUART);

    // Wait for reception of a character on the USART RX line and echo this
    // character on console 
    HAL_UART_Receive(g_HDebugUART, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    //HAL_UART_Transmit(g_HDebugUART, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;    
*/
}

// ---------------------------------------------------------
// 下面是 Shell 需要的三个函数实现
// ---------------------------------------------------------

/**
 * @brief 发送一个字符 (阻塞模式)
 * 对应原代码的 putchar
   发送shell的命令调试信息用的(只用于)
 */
int shell_putchar(int c)
{
    // 使用 HAL_MAX_DELAY 模拟原代码中的 while(...) 等待行为
    HAL_UART_Transmit(g_HDebugUART, (uint8_t *)&c, 1, HAL_MAX_DELAY);

    // 2. 发给 ESP8266 (新增)
    //HAL_UART_Transmit(&huart_esp8266, (uint8_t *)&c, 1, 10);

//	if(getSn_SR(SOCK_TCPS) == SOCK_ESTABLISHED)
//	{
//		//HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
//		send(SOCK_TCPS,(const uint8_t *)&c,1);
//		//HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
//	}

//    if(g_WiFi_Shell_Enable)
//    {
//        if(g_esp_ptr < ESP_LINE_BUF_SIZE)
//        {
//            g_esp_buf[g_esp_ptr++] = (uint8_t)c;
//        }
//        
//        if(c == '\n' || c == '\r' || g_esp_ptr >= ESP_LINE_BUF_SIZE-1)
//        {
//            esp_flush_buffer();
//        }
//    }

    // 3. 新增：发给 W5500 TCP 客户端
//	if(getSn_SR(SOCK_TCPS) == SOCK_ESTABLISHED)
//	{
//		HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
//		send(SOCK_TCPS,(const uint8_t *)&c,1);
//		HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
//	}
//    if(g_w5500_ptr < W5500_LINE_BUF_SIZE)
//    {
//        g_w5500_buf[g_w5500_ptr++] = c;
//    }
//    if(c == '\n' || c == '\r' || g_w5500_ptr >= W5500_LINE_BUF_SIZE)//
//    {
//        w5500_flush_buffer();
//    }
    return c;
}

/**
 * @brief 接收一个字符 (阻塞模式)
 * 对应原代码的 getchar
 */
int shell_getchar(void)
{
	return getchar2();
/*
    uint8_t ch = 0;
    
    // 清除溢出标志，防止之前的数据导致接收卡死
    __HAL_UART_CLEAR_OREFLAG(g_HDebugUART);
    
    // 接收1个字节，HAL_MAX_DELAY 表示一直等待直到收到数据
    if (HAL_UART_Receive(g_HDebugUART, &ch, 1, HAL_MAX_DELAY) == HAL_OK)
    {
        return (int)ch;
    }
    else
    {
        return -1; // 错误情况
    }
*/
}
int getchar2(void)
{
    unsigned char c;
    while (0 != ring_buffer_read(&c, &rx_buf));
    return c;
}

/**
 * @brief 发送字符串
 * 对应原代码的 putstr
 */
void putstr(const char *str)
{
    while (*str)
    {
        putchar(*str);
        str++;
    }
}

void putdatas(const char *datas, int len)
{
	int i;
	for (i = 0; i < len; i++)
	{
		putchar(datas[i]);
	}
}

/* USER CODE END 1 */
