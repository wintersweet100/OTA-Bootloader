/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32f1xx_it.c
 * @brief   Interrupt Service Routines.
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
#include "stm32f1xx_it.h"
#include "main.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bsp_esp8266.h"
#include "ring_buffer.h"
#include "string.h"
#include "tim.h"
#include <string.h>
#include "FileTransferReceiver.h"
#include "usart.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim6;
extern UART_HandleTypeDef huart1;
/* USER CODE BEGIN EV */
extern ring_buffer rx_buf; // 确保声明了 Shell 用的环形缓冲区
extern uint8_t g_WiFi_Shell_Enable;
/* 外部变量声明 */
extern UART_HandleTypeDef huart_esp8266;
extern struct STRUCT_USARTx_Fram strEsp8266_Fram_Record;
extern volatile uint8_t ucTcpClosedFlag;
extern FileReceiver fileReceiver;

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M3 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
 * @brief This function handles Non maskable interrupt.
 */
void NMI_Handler(void) {
    /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

    /* USER CODE END NonMaskableInt_IRQn 0 */
    /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
    while (1) {
    }
    /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
 * @brief This function handles Hard fault interrupt.
 */
void HardFault_Handler(void) {
    /* USER CODE BEGIN HardFault_IRQn 0 */

    /* USER CODE END HardFault_IRQn 0 */
    while (1) {
        /* USER CODE BEGIN W1_HardFault_IRQn 0 */
        /* USER CODE END W1_HardFault_IRQn 0 */
    }
}

/**
 * @brief This function handles Memory management fault.
 */
void MemManage_Handler(void) {
    /* USER CODE BEGIN MemoryManagement_IRQn 0 */

    /* USER CODE END MemoryManagement_IRQn 0 */
    while (1) {
        /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
        /* USER CODE END W1_MemoryManagement_IRQn 0 */
    }
}

/**
 * @brief This function handles Prefetch fault, memory access fault.
 */
void BusFault_Handler(void) {
    /* USER CODE BEGIN BusFault_IRQn 0 */

    /* USER CODE END BusFault_IRQn 0 */
    while (1) {
        /* USER CODE BEGIN W1_BusFault_IRQn 0 */
        /* USER CODE END W1_BusFault_IRQn 0 */
    }
}

/**
 * @brief This function handles Undefined instruction or illegal state.
 */
void UsageFault_Handler(void) {
    /* USER CODE BEGIN UsageFault_IRQn 0 */

    /* USER CODE END UsageFault_IRQn 0 */
    while (1) {
        /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
        /* USER CODE END W1_UsageFault_IRQn 0 */
    }
}

/**
 * @brief This function handles System service call via SWI instruction.
 */
void SVC_Handler(void) {
    /* USER CODE BEGIN SVCall_IRQn 0 */

    /* USER CODE END SVCall_IRQn 0 */
    /* USER CODE BEGIN SVCall_IRQn 1 */

    /* USER CODE END SVCall_IRQn 1 */
}

/**
 * @brief This function handles Debug monitor.
 */
void DebugMon_Handler(void) {
    /* USER CODE BEGIN DebugMonitor_IRQn 0 */

    /* USER CODE END DebugMonitor_IRQn 0 */
    /* USER CODE BEGIN DebugMonitor_IRQn 1 */

    /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
 * @brief This function handles Pendable request for system service.
 */
void PendSV_Handler(void) {
    /* USER CODE BEGIN PendSV_IRQn 0 */

    /* USER CODE END PendSV_IRQn 0 */
    /* USER CODE BEGIN PendSV_IRQn 1 */

    /* USER CODE END PendSV_IRQn 1 */
}

/**
 * @brief This function handles System tick timer.
 */
void SysTick_Handler(void) {
    /* USER CODE BEGIN SysTick_IRQn 0 */

    /* USER CODE END SysTick_IRQn 0 */
    HAL_IncTick();
    /* USER CODE BEGIN SysTick_IRQn 1 */

    /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F1xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f1xx.s).                    */
/******************************************************************************/

/**
 * @brief This function handles EXTI line[9:5] interrupts.
 */
void EXTI9_5_IRQHandler(void) {
    /* USER CODE BEGIN EXTI9_5_IRQn 0 */
    /* USER CODE END EXTI9_5_IRQn 0 */
    HAL_GPIO_EXTI_IRQHandler(W5500_INT_Pin);
    /* USER CODE BEGIN EXTI9_5_IRQn 1 */

    /* USER CODE END EXTI9_5_IRQn 1 */
}

/**
 * @brief This function handles USART1 global interrupt.
 */
void USART1_IRQHandler(void) {
    /* USER CODE BEGIN USART1_IRQn 0 */
    USART_TypeDef *usart1 = (USART_TypeDef *)0x40013800;

    // 检查溢出错误
    if (usart1->SR & (1 << 3)) {
        // 错误处理
        usart1->SR &= ~(1 << 3); // 清除错误标志
    }

    // 检查接收数据
    if (usart1->SR & (1 << 5)) {
        ring_buffer_write(usart1->DR, &rx_buf);
    }
    return; // 直接返回,不调用HAL处理
            /* USER CODE END USART1_IRQn 0 */
            // HAL_UART_IRQHandler(&huart1);
    /* USER CODE BEGIN USART1_IRQn 1 */

    /* USER CODE END USART1_IRQn 1 */
}

/**
 * @brief This function handles TIM6 global interrupt.
 */
void TIM6_IRQHandler(void) {
    /* USER CODE BEGIN TIM6_IRQn 0 */
    TIM6_IncTick();
    /* USER CODE END TIM6_IRQn 0 */
    HAL_TIM_IRQHandler(&htim6);
    /* USER CODE BEGIN TIM6_IRQn 1 */

    /* USER CODE END TIM6_IRQn 1 */
}

/* USER CODE BEGIN 1 */
/* 在 stm32f1xx_it.c 中修改 */

static uint8_t  wifi_frame_buf[2048];  // WiFi帧缓冲区
static uint16_t wifi_frame_pos = 0;    // 缓冲区位置

#if 0
void USART3_IRQHandler(void)
{
    uint8_t ucCh;
    USART_TypeDef *usart3 = USART3;
    static uint8_t  state = 0;
    static uint8_t  match_idx = 0;
    static uint32_t rem_len = 0;
    
    if (usart3->SR & UART_FLAG_RXNE)
    {
        ucCh = (uint8_t)(usart3->DR & 0x00FF);
        
        // 原始记录
        if (strEsp8266_Fram_Record.InfBit.FramLength < (RX_BUF_MAX_LEN - 1))
        {
            strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength++] = ucCh;
        }
        
        if (g_WiFi_Shell_Enable == 0)
        {
            ring_buffer_write(ucCh, &rx_buf);
        }
        else
        {
            // ★ 改进：逐字节处理，而不是等待整个IPD包
            switch (state)
            {
                case 0: // 搜索 "+IPD"
                    if (ucCh == "+IPD"[match_idx]) {
                        match_idx++;
                        if (match_idx == 4) {
                            state = 1;
                            match_idx = 0;
                            rem_len = 0;
                            wifi_frame_pos = 0;
                        }
                    } else {
                        if (ucCh == '+') {
                            match_idx = 1;
                        } else if (match_idx == 1 && ucCh == 'I') {
                            match_idx = 2;
                        } else if (match_idx == 2 && ucCh == 'P') {
                            match_idx = 3;
                        } else {
                            match_idx = 0;
                        }
                    }
                    break;
                    
                case 1: // 解析长度
                    if (ucCh >= '0' && ucCh <= '9') {
                        rem_len = rem_len * 10 + (ucCh - '0');
                    } else if (ucCh == ':') {
                        if (rem_len > 0) {
                            state = 2;
                            wifi_frame_pos = 0;
                        } else {
                            state = 0;
                            match_idx = 0;
                        }
                    } else if (ucCh == ',') {
                        rem_len = 0;
                    }
                    break;
                    
                case 2: // ★ 逐字节接收数据，边接收边处理
                    if (wifi_frame_pos < sizeof(wifi_frame_buf)) {
                        wifi_frame_buf[wifi_frame_pos++] = ucCh;
                    }
                    rem_len--;
                    
                    // ★ 关键改进：逐字节尝试解析帧（参考W5500逻辑）
                    if (wifi_frame_pos >= 7) {
                        // 检查是否是文件传输帧头
                        if (wifi_frame_buf[0] == 0xAA) {
                            // 尝试解析帧
                            Frame frame;
                            if (parseFrame(wifi_frame_buf, wifi_frame_pos, &frame)) {
                                uint16_t frameLen = 1 + 1 + 2 + frame.len + 2 + 1;
                                
                                // 处理帧
                                processFrame(&fileReceiver, &frame);
                                
                                // 移除已处理的帧
                                wifi_frame_pos -= frameLen;
                                if (wifi_frame_pos > 0) {
                                    memmove(wifi_frame_buf, &wifi_frame_buf[frameLen], wifi_frame_pos);
                                }
                                // 继续处理下一个帧（可能在缓冲区中）
                            }
                        } else {
                            // 不是文件传输帧，写入 rx_buf
                            ring_buffer_write(wifi_frame_buf[0], &rx_buf);
                            wifi_frame_pos--;
                            if (wifi_frame_pos > 0) {
                                memmove(wifi_frame_buf, &wifi_frame_buf[1], wifi_frame_pos);
                            }
                        }
                    }
                    
                    if (rem_len == 0) {
                        // IPD包接收完毕
                        if (wifi_frame_pos > 0 && wifi_frame_buf[0] != 0xAA) {
                            // 剩余的非文件传输数据写入 rx_buf
                            for (uint16_t i = 0; i < wifi_frame_pos; i++) {
                                ring_buffer_write(wifi_frame_buf[i], &rx_buf);
                            }
                        }
                        state = 0;
                        match_idx = 0;
                        wifi_frame_pos = 0;
                    }
                    break;
                    
                default:
                    state = 0;
                    match_idx = 0;
                    rem_len = 0;
                    wifi_frame_pos = 0;
                    break;
            }
        }
    }
    
    if (usart3->SR & UART_FLAG_IDLE)
    {
        __HAL_UART_CLEAR_IDLEFLAG(&huart_esp8266);
        strEsp8266_Fram_Record.InfBit.FramFinishFlag = 1;
        strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength] = '\0';
        
        if (strstr((char*)strEsp8266_Fram_Record.Data_RX_BUF, "CLOSED\r\n")) {
            ucTcpClosedFlag = 1;
        }
        
        if (g_WiFi_Shell_Enable != 0 && state != 2) {
            state = 0;
            match_idx = 0;
            rem_len = 0;
            wifi_frame_pos = 0;
        }
    }
}
#endif

#if 1
/*
逻辑流程
	case 0：只搜索 +IPD，不匹配就丢弃
	case 1：解析长度字段
	case 2：接收完整帧数据后，判断：
		帧头是 0xAA → 文件传输帧，调用 fileReceiverHandleData()
		否则 → shell命令，写入 rx_buf
*/

void USART3_IRQHandler(void)
{
    uint8_t ucCh;
    USART_TypeDef *usart3 = USART3;
    
    // 状态机变量：仅在 g_WiFi_Shell_Enable = 1 时生效
    static uint8_t  state = 0;       // 0:找+IPD, 1:解析长度, 2:接收纯数据
    static uint8_t  match_idx = 0;
    static uint32_t rem_len = 0;     // 记录当前 IPD 包剩余需要接收的长度


    // 1. 处理接收中断 (RXNE)
    if (usart3->SR & UART_FLAG_RXNE)
    {
        ucCh = (uint8_t)(usart3->DR & 0x00FF);
        
        // --- 原始记录：无论什么模式，始终记录到系统缓冲区，用于掉线检测 ---
        if (strEsp8266_Fram_Record.InfBit.FramLength < (RX_BUF_MAX_LEN - 1))
        {
            strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength++] = ucCh;
        }

        // --- 分模逻辑：解决初始化与数据传输的冲突 ---
        if (g_WiFi_Shell_Enable == 0) 
        {
            // 【模式 A：初始化模式】所有数据（OK, ready, AT）全部直接进入 rx_buf [3]
            // 这确保了初始化程序能读到 AT 指令响应，不会卡死
            ring_buffer_write(ucCh, &rx_buf);
        }
        else 
        {

            // 【模式 B：Shell/数据传输模式】开启状态机剥离 +IPD 头部 [1]
            switch (state)
            {
                case 0: // 搜索 "+IPD" 头部
                    if (ucCh == "+IPD"[match_idx]) {
                        match_idx++;
//						printf("[UART3] Matched '+IPD'[%d]\r\n", match_idx - 1);
		
                        if (match_idx == 4) {	
                            state = 1;      // 匹配成功
                            match_idx = 0;
                            rem_len = 0;    // 准备解析长度
							wifi_frame_pos = 0;  // 重置帧缓冲区
							//printf("[UART3] Found '+IPD', entering state 1 (parse length)\r\n");
                        }
                    } else {
	//                        // 如果不是 +IPD，判断是否为回显（可选：非 IPD 数据也可根据需求写入 rx_buf）
	//                        match_idx = (ucCh == '+') ? 1 : 0;
						// ★ 只有在看到 '+' 后面紧跟 'I' 时才重置
						if (ucCh == '+') {
							match_idx = 1;
						} else if (match_idx == 1 && ucCh == 'I') {
							match_idx = 2;
						} else if (match_idx == 2 && ucCh == 'P') {
							match_idx = 3;
						} else {
							match_idx = 0;
						}
                    }
                    break;

                case 1: // 解析长度字段（格式如：,0,1024:）
                    if (ucCh >= '0' && ucCh <= '9') {
                        rem_len = rem_len * 10 + (ucCh - '0');
//						printf("[UART3] Parsing length: rem_len=%d\r\n", rem_len);

                    } else if (ucCh == ':') {
                        if (rem_len > 0){
							state = 2; // 解析到冒号，且长度有效，进入提取模式
//							printf("[UART3] Length parsed: %d bytes, entering state 2 (receive data)\r\n", rem_len);

                        }else {
							state = 0;             // 长度无效，复位
							match_idx = 0;
//							 printf("[UART3] Invalid length, reset to state 0\r\n");
						}
                    } else if (ucCh == ',') {
                        rem_len = 0; // 遇到逗号重置长度累加（跳过前面的 ID 字段）
//						printf("[UART3] Found comma, reset rem_len\r\n");

                    }
                    break;

                case 2: 
	                // 提取纯净数据只写入帧缓冲区，不写入 rx_buf
                    if (wifi_frame_pos < sizeof(wifi_frame_buf)) {
                        wifi_frame_buf[wifi_frame_pos++] = ucCh;
                    }
                    rem_len--;

//                    // ★ 每接收 10 字节打印一次进度
//                    if (wifi_frame_pos % 10 == 0) {
//                        printf("[UART3] Receiving data: %d/%d bytes\r\n", wifi_frame_pos, wifi_frame_pos + rem_len);
//                    }


					if(rem_len == 0){//一个完整的帧已接收

//                        frame_count++;
//                        printf("[UART3] Frame complete! Frame #%d, size=%d bytes\r\n", frame_count, wifi_frame_pos);
//                        
//                        // ★ 打印完整帧的十六进制
//                        printf("[UART3] Frame hex: ");
//                        for (uint16_t i = 0; i < wifi_frame_pos && i < 50; i++) {
//                            printf("%02X ", wifi_frame_buf[i]);
//                        }
//                        if (wifi_frame_pos > 50) {
//                            printf("... (total %d bytes)", wifi_frame_pos);
//                        }
//                        printf("\r\n");


						// 检测是否是文件传输协议帧（帧头 0xAA）
                        if (wifi_frame_pos >= 2 && wifi_frame_buf[0] == 0xAA) {

//                            printf("[UART3] File transfer frame detected (0xAA)\r\n");
//                            printf("[UART3] Frame type: 0x%02X\r\n", wifi_frame_buf[1]);

                            // 文件传输帧，调用文件传输处理函数
                            fileReceiverHandleData(&fileReceiver, wifi_frame_buf, wifi_frame_pos);
                        } else {

//                            printf("[UART3] Shell command frame\r\n");

                            // 普通数据，写入 rx_buf（shell命令）
                            for (uint16_t i = 0; i < wifi_frame_pos; i++) {
                                ring_buffer_write(wifi_frame_buf[i], &rx_buf);
                            }
                        }

                        state = 0;		// 重点：一旦接收完指定长度，立即回到搜索模式，剥离下一个包的头部
                        match_idx = 0;
                        wifi_frame_pos = 0;						
					}
                    break;
			    default:
                    state = 0;
                    match_idx = 0;
                    rem_len = 0;
                    wifi_frame_pos = 0;	
                    break;
            }
        }
    }

    // 2. 处理空闲中断 (IDLE)
    if (usart3->SR & UART_FLAG_IDLE)
    {
        // 清除 IDLE 标志 [1]
        __HAL_UART_CLEAR_IDLEFLAG(&huart_esp8266);
        
        strEsp8266_Fram_Record.InfBit.FramFinishFlag = 1;
        strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength] = '\0';

//        printf("[UART3] IDLE interrupt, total received: %d bytes, frames: %d\r\n", 
//               total_received, frame_count);

        
        // 检测 TCP 链接是否关闭
        if (strstr((char*)strEsp8266_Fram_Record.Data_RX_BUF, "CLOSED\r\n")) {
            ucTcpClosedFlag = 1;

//            printf("[UART3] TCP connection closed\r\n");
        }
        
        /* 
         * 注意：不要在这里执行 strEsp8266_Fram_Record.InfBit.FramLength = 0;
         * 长度清零应由 main 函数中处理完 AT 指令响应后手动执行，
         * 否则初始化程序会因为数据被提前清空而认为没收到 "OK" [3]。
         */
        
        // 传输阶段非数据接收态时，重置状态机
        if (g_WiFi_Shell_Enable != 0 && state != 2) {

            state = 0;
            match_idx = 0;
			rem_len = 0;          
            wifi_frame_pos = 0;                          
        }
    }
}
#endif

//可以使用wifi进行命令交互的版本，但是不能接收.bin文件
#if 0
void USART3_IRQHandler(void)
{
    uint8_t ucCh;
    USART_TypeDef *usart3 = USART3;
    
    // 状态机变量：仅在 g_WiFi_Shell_Enable = 1 时生效
    static uint8_t  state = 0;       // 0:找+IPD, 1:解析长度, 2:接收纯数据
    static uint8_t  match_idx = 0;
    static uint32_t rem_len = 0;     // 记录当前 IPD 包剩余需要接收的长度

    // 1. 处理接收中断 (RXNE)
    if (usart3->SR & UART_FLAG_RXNE)
    {
        ucCh = (uint8_t)(usart3->DR & 0x00FF);
        
        // --- 原始记录：无论什么模式，始终记录到系统缓冲区，用于掉线检测 ---
        if (strEsp8266_Fram_Record.InfBit.FramLength < (RX_BUF_MAX_LEN - 1))
        {
            strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength++] = ucCh;
        }

        // --- 分模逻辑：解决初始化与数据传输的冲突 ---
        if (g_WiFi_Shell_Enable == 0) 
        {
            // 【模式 A：初始化模式】所有数据（OK, ready, AT）全部直接进入 rx_buf [3]
            // 这确保了初始化程序能读到 AT 指令响应，不会卡死
            ring_buffer_write(ucCh, &rx_buf);
        }
        else 
        {
            // 【模式 B：Shell/数据传输模式】开启状态机剥离 +IPD 头部 [1]
            switch (state)
            {
                case 0: // 搜索 "+IPD" 头部
                    if (ucCh == "+IPD"[match_idx]) {
                        match_idx++;
                        if (match_idx == 4) {	
                            state = 1;      // 匹配成功
                            match_idx = 0;
                            rem_len = 0;    // 准备解析长度
                        }
                    } else {
                        // 如果不是 +IPD，判断是否为回显（可选：非 IPD 数据也可根据需求写入 rx_buf）
                        match_idx = (ucCh == '+') ? 1 : 0;
                    }
                    break;

                case 1: // 解析长度字段（格式如：,0,1024:）
                    if (ucCh >= '0' && ucCh <= '9') {
                        rem_len = rem_len * 10 + (ucCh - '0');
                    } else if (ucCh == ':') {
                        if (rem_len > 0){
							state = 2; // 解析到冒号，且长度有效，进入提取模式
                        }else {
							state = 0;             // 长度无效，复位
							match_idx = 0;
						}
                    } else if (ucCh == ',') {
                        rem_len = 0; // 遇到逗号重置长度累加（跳过前面的 ID 字段）
                    }
                    break;

                case 2: // 提取纯净数据写入 rx_buf
                    ring_buffer_write(ucCh, &rx_buf);
                    rem_len--;
                    if (rem_len == 0) {
                        state = 0; // 重点：一旦接收完指定长度，立即回到搜索模式，剥离下一个包的头部 [1]
						match_idx = 0;
                    }
                    break;
			    default:
                    state = 0;
                    match_idx = 0;
                    rem_len = 0;
                    break;
            }
        }
    }

    // 2. 处理空闲中断 (IDLE)
    if (usart3->SR & UART_FLAG_IDLE)
    {
        // 清除 IDLE 标志 [1]
        __HAL_UART_CLEAR_IDLEFLAG(&huart_esp8266);
        
        strEsp8266_Fram_Record.InfBit.FramFinishFlag = 1;
        strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength] = '\0';
        
        // 检测 TCP 链接是否关闭
        if (strstr((char*)strEsp8266_Fram_Record.Data_RX_BUF, "CLOSED\r\n")) {
            ucTcpClosedFlag = 1;
        }
        
        /* 
         * 注意：不要在这里执行 strEsp8266_Fram_Record.InfBit.FramLength = 0;
         * 长度清零应由 main 函数中处理完 AT 指令响应后手动执行，
         * 否则初始化程序会因为数据被提前清空而认为没收到 "OK" [3]。
         */
        
        // 传输阶段非数据接收态时，重置状态机
        if (g_WiFi_Shell_Enable != 0 && state != 2) {
            state = 0;
            match_idx = 0;
			rem_len = 0;                                    
        }
    }
}
#endif

#if 0
	//这个函数可以将UART3与shell交互起来，但是仅限于透传模式(STA+Client)
	//但是我现在要用ESP做Server
void USART3_IRQHandler(void)
{
    uint8_t ucCh;
    USART_TypeDef *usart3 = USART3;

    // 1. 处理接收中断 (RXNE)
    if (usart3->SR & UART_FLAG_RXNE)
    {
        ucCh = (uint8_t)(usart3->DR & 0x00FF);
        
        // --- 核心修改：将数据注入 Shell 的缓冲区 ---
        // 这样 shell() 函数里的 getchar2 就能读到来自 WiFi 的数据了
        ring_buffer_write(ucCh, &rx_buf);
        // ----------------------------------------

        // 原有的记录逻辑（如果还需要 CheckRecvDataTest 做掉线检测的话保留）
        if (strEsp8266_Fram_Record.InfBit.FramLength < (RX_BUF_MAX_LEN - 1))
        {
            strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength++] = ucCh;
        }
    }

    // 2. 处理空闲中断 (IDLE)
    if (usart3->SR & UART_FLAG_IDLE)
    {
        __HAL_UART_CLEAR_IDLEFLAG(&huart_esp8266);
        strEsp8266_Fram_Record.InfBit.FramFinishFlag = 1;
        strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength] = '\0';
        
        if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "CLOSED\r\n"))
        {
            ucTcpClosedFlag = 1;
        }
    }
}
#endif

#if 0
//下面这个函数可以通过ESP8266-UART3与UART1调试串口的交互，上面新的函数可以将UART3与shell交互起来
void USART3_IRQHandler(void)
{
    uint8_t ucCh;
    USART_TypeDef *usart3 = USART3;

    // 1. 处理接收中断 (RXNE)
    if (usart3->SR & UART_FLAG_RXNE)
    {
        ucCh = (uint8_t)(usart3->DR & 0x00FF);
        if (strEsp8266_Fram_Record.InfBit.FramLength < (RX_BUF_MAX_LEN - 1))
        {
            strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength++] = ucCh;
        }
    }

    // 2. 处理空闲中断 (IDLE) - 用于判断一帧数据接收完毕
    if (usart3->SR & UART_FLAG_IDLE)
    {
        // HAL 库清除 IDLE 标志的方法：先读 SR 再读 DR
        __HAL_UART_CLEAR_IDLEFLAG(&huart_esp8266);
        
        strEsp8266_Fram_Record.InfBit.FramFinishFlag = 1;
        strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength] = '\0';
        
        // 检查掉线标志
        if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "CLOSED\r\n"))
        {
            ucTcpClosedFlag = 1;
        }
    }
}
#endif
/* USER CODE END 1 */
