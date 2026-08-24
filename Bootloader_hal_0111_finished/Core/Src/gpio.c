/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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
#include "gpio.h"

/* USER CODE BEGIN 0 */
#include "socket.h"
#include "ring_buffer.h"
#include "w5500.h"
#include "FileTransferReceiver.h"
#include "usart.h"
/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */
extern uint8 buff[2048];	
extern ring_buffer rx_buf;
extern FileReceiver fileReceiver;
/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_Blue_GPIO_Port, LED_Blue_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, W5500_CS_Pin|W5500_RST_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : LED_Blue_Pin */
  GPIO_InitStruct.Pin = LED_Blue_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(LED_Blue_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : W5500_INT_Pin */
  GPIO_InitStruct.Pin = W5500_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(W5500_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : W5500_CS_Pin W5500_RST_Pin */
  GPIO_InitStruct.Pin = W5500_CS_Pin|W5500_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 2 */
/**
  * @brief  重写 EXTI 回调函数
  * @param  GPIO_Pin: 触发中断的引脚
  */
#if 1 
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == W5500_INT_Pin)
    {
        // 1. 读取中断寄存器 Sn_IR，而不是状态寄存器 Sn_SR
        uint8_t ir = getSn_IR(SOCK_TCPS); 

        // 2. 检查是否有接收中断
        if (ir & Sn_IR_RECV)
        {
            uint16_t len = getSn_RX_RSR(SOCK_TCPS);
            if (len > 0)
            {
                // 限制单次读取长度，防止 buff 溢出
                if (len > 2048) len = 2048; 
               
                recv(SOCK_TCPS, buff, len);
                
				// ★ 添加：打印收到的QT发来的帧数据----测试用
//				printf("[W5500 RECV] Received %d bytes: ", len);
//				for (uint16_t i = 0; i < (len < 20 ? len : 20); i++) {
//					printf("%02X ", buff[i]);
//				}
//				printf("\n");

                // ★ 检测是否是文件传输协议帧（帧头 0xAA）
                if (len >= 2 && buff[0] == 0xAA) {

//					printf("[W5500] Detected frame type: 0x%02X\n",buff[1]);

//                    // 如果是首次接收文件帧，发送READY
//                    if (fileReceiver.state == RECV_IDLE) {
//					printf("[W5500] Sending READY...\n");
//                        fileReceiverSendReady(&fileReceiver);
//                        fileReceiver.state = RECV_WAITING_FILE_START;
//                    }
                    
                    // 处理文件传输数据
                    fileReceiverHandleData_W5500(&fileReceiver, buff, len);
                } 
				else{
					// 批量写入 ring_buffer，如果你的 ring_buffer 支持 memcpy 更好
					for (uint16_t i = 0; i < len; i++)
					{
						ring_buffer_write(buff[i], &rx_buf);
					}
				}

			}
            // 写 1 清除接收中断位
            setSn_IR(SOCK_TCPS, Sn_IR_RECV); 
        }

        // 3. 检查是否有连接中断
        if (ir & Sn_IR_CON)
        {
            setSn_IR(SOCK_TCPS, Sn_IR_CON); 
        }

        // 4. 处理其他中断（如超时、断开等），防止 INTn 引脚始终拉低
        if (ir & Sn_IR_DISCON || ir & Sn_IR_TIMEOUT)
        {
            setSn_IR(SOCK_TCPS, 0xFF); // 清除所有余下中断
        }
    }
}
#endif

//W5500--使用Z-modem时的版本，因传输问题，不明原因放弃
#if 0
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == W5500_INT_Pin)
    {
        // 1. 读取中断寄存器 Sn_IR，而不是状态寄存器 Sn_SR
        uint8_t ir = getSn_IR(SOCK_TCPS); 

        // 2. 检查是否有接收中断
        if (ir & Sn_IR_RECV)
        {
            uint16_t len = getSn_RX_RSR(SOCK_TCPS);
            if (len > 0)
            {
                // 限制单次读取长度，防止 buff 溢出
                if (len > 2048) len = 2048; 
                
                recv(SOCK_TCPS, buff, len);
                
                // 批量写入 ring_buffer，如果你的 ring_buffer 支持 memcpy 更好
                for (uint16_t i = 0; i < len; i++)
                {
                    ring_buffer_write(buff[i], &rx_buf);
                }
            }
            // 写 1 清除接收中断位
            setSn_IR(SOCK_TCPS, Sn_IR_RECV); 
        }

        // 3. 检查是否有连接中断
        if (ir & Sn_IR_CON)
        {
            setSn_IR(SOCK_TCPS, Sn_IR_CON); 
        }

        // 4. 处理其他中断（如超时、断开等），防止 INTn 引脚始终拉低
        if (ir & Sn_IR_DISCON || ir & Sn_IR_TIMEOUT)
        {
            setSn_IR(SOCK_TCPS, 0xFF); // 清除所有余下中断
        }
    }
}
#endif

#if 0
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint16 len = 0;
    // 这里的逻辑会覆盖驱动文件里的空函数
    if (GPIO_Pin == W5500_INT_Pin) // 假设你在CubeMX里给PG8起名叫W5500_INT
    {
		uint8_t ir = getSn_SR(SOCK_TCPS);
		
		if( ir == SOCK_ESTABLISHED)
		{
            // 只有当有数据时才读取
            len = getSn_RX_RSR(SOCK_TCPS);
            if(len > 0)
            {
                // 读取数据到临时缓存 buff
                recv(SOCK_TCPS, buff, len); 
                // 将数据逐个写入 Shell 的环形缓冲区
                for(uint16_t i = 0; i < len; i++)
                {
                    ring_buffer_write(buff[i], &rx_buf);
					printf("%02X ",buff[i]);
                }
				// --- 核心关键：写1清掉 W5500 内部的接收中断位 ---
				// 只有清掉了这一位，W5500 才会释放 INTn 引脚（恢复高电平）
				setSn_IR(SOCK_TCPS, Sn_IR_RECV); 
            }
		}
        // 3. 如果是连接中断 (PC刚连上来时会触发)
        if(ir == Sn_IR_CON)
        {
            setSn_IR(SOCK_TCPS, Sn_IR_CON); // 清除连接中断
        }
    }
}
#endif
/* USER CODE END 2 */
