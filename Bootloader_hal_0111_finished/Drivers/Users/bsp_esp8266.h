#ifndef  __BSP_ESP8266_H
#define	 __BSP_ESP8266_H

#include "stm32f1xx.h"
#include "stm32f1xx_hal_gpio.h"
#include <stdbool.h>



#if defined ( __CC_ARM   )
#pragma anon_unions
#endif

/*!< STM32F10x Standard Peripheral Library old types (maintained for legacy purpose) 
	STM32F10x标准外设库旧类型(为遗留问题而维护)*/
typedef int32_t  s32;
typedef int16_t s16;
typedef int8_t  s8;

typedef const int32_t sc32;  /*!< Read Only */
typedef const int16_t sc16;  /*!< Read Only */
typedef const int8_t sc8;   /*!< Read Only */

typedef __IO int32_t  vs32;
typedef __IO int16_t  vs16;
typedef __IO int8_t   vs8;

typedef __I int32_t vsc32;  /*!< Read Only */
typedef __I int16_t vsc16;  /*!< Read Onl	y */
typedef __I int8_t vsc8;   /*!< Read Only */

typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef const uint32_t uc32;  /*!< Read Only */
typedef const uint16_t uc16;  /*!< Read Only */
typedef const uint8_t uc8;   /*!< Read Only */

typedef __IO uint32_t  vu32;
typedef __IO uint16_t vu16;
typedef __IO uint8_t  vu8;

typedef __I uint32_t vuc32;  /*!< Read Only */
typedef __I uint16_t vuc16;  /*!< Read Only */
typedef __I uint8_t vuc8;   /*!< Read Only */



/******************************* ESP8266 数据类型定义 ***************************/
typedef enum{
	STA,
  AP,
  STA_AP  
} ENUM_Net_ModeTypeDef;


typedef enum{
	 enumTCP,
	 enumUDP,
} ENUM_NetPro_TypeDef;
	

typedef enum{
	Multiple_ID_0 = 0,
	Multiple_ID_1 = 1,
	Multiple_ID_2 = 2,
	Multiple_ID_3 = 3,
	Multiple_ID_4 = 4,
	Single_ID_0 = 5,
} ENUM_ID_NO_TypeDef;
	

typedef enum{
	OPEN = 0,
	WEP = 1,
	WPA_PSK = 2,
	WPA2_PSK = 3,
	WPA_WPA2_PSK = 4,
} ENUM_AP_PsdMode_TypeDef;



/******************************* ESP8266 外部全局变量声明 ***************************/
#define RX_BUF_MAX_LEN     1024                                     //最大接收缓存字节数

extern struct  STRUCT_USARTx_Fram                                  //串口数据帧的处理结构体
{
	char  Data_RX_BUF [ RX_BUF_MAX_LEN ];//接收的数据缓冲区
	
	union {
		__IO u16 InfAll;		//// 整体作为一个16位无符号整数访问
		struct {
			  __IO u16 FramLength       :15;      // 14:0 // 低15位：帧长度（0-32767字节）
			  __IO u16 FramFinishFlag   :1;       // 15   // 第15位：帧完成标志（0/1）
		  } InfBit;
	  }; 
	
} strEsp8266_Fram_Record;

extern struct STRUCT_USARTx_Fram strUSART_Fram_Record;

void USART_printf ( USART_TypeDef * USARTx, char * Data, ... );
static char * itoa( int value, char *string, int radix );
/******************************** ESP8266 连接引脚定义 ***********************************/
#define      macESP8266_CH_PD_APBxClock_FUN                   RCC_APB2PeriphClockCmd
#define      macESP8266_CH_PD_CLK                             RCC_APB2Periph_GPIOG  
#define      macESP8266_CH_PD_PORT                            GPIOG
#define      macESP8266_CH_PD_PIN                             GPIO_PIN_13

#define      macESP8266_RST_APBxClock_FUN                     RCC_APB2PeriphClockCmd
#define      macESP8266_RST_CLK                               RCC_APB2Periph_GPIOG
#define      macESP8266_RST_PORT                              GPIOG
#define      macESP8266_RST_PIN                               GPIO_PIN_14

 

#define      macESP8266_USART_BAUD_RATE                       115200

#define      macESP8266_USARTx                                USART3
#define      macESP8266_USART_APBxClock_FUN                   RCC_APB1PeriphClockCmd
#define      macESP8266_USART_CLK                             RCC_APB1Periph_USART3
#define      macESP8266_USART_GPIO_APBxClock_FUN              RCC_APB2PeriphClockCmd
#define      macESP8266_USART_GPIO_CLK                        RCC_APB2Periph_GPIOB     
#define      macESP8266_USART_TX_PORT                         GPIOB   
#define      macESP8266_USART_TX_PIN                          GPIO_PIN_10
#define      macESP8266_USART_RX_PORT                         GPIOB
#define      macESP8266_USART_RX_PIN                          GPIO_PIN_11
#define      macESP8266_USART_IRQ                             USART3_IRQn
#define      macESP8266_USART_INT_FUN                         USART3_IRQHandler



/*********************************************** ESP8266 函数宏定义 *******************************************/
#define     macESP8266_Usart( fmt, ... )           USART_printf ( macESP8266_USARTx, fmt, ##__VA_ARGS__ ) 
#define     macPC_Usart( fmt, ... )                printf ( fmt, ##__VA_ARGS__ )
//#define     macPC_Usart( fmt, ... )                

#define     macESP8266_CH_ENABLE()                 HAL_GPIO_WritePin ( macESP8266_CH_PD_PORT, macESP8266_CH_PD_PIN ,GPIO_PIN_SET)
#define     macESP8266_CH_DISABLE()                HAL_GPIO_WritePin ( macESP8266_CH_PD_PORT, macESP8266_CH_PD_PIN ,GPIO_PIN_RESET)

#define     macESP8266_RST_HIGH_LEVEL()            HAL_GPIO_WritePin ( macESP8266_RST_PORT, macESP8266_RST_PIN ,GPIO_PIN_SET)
#define     macESP8266_RST_LOW_LEVEL()             HAL_GPIO_WritePin ( macESP8266_RST_PORT, macESP8266_RST_PIN ,GPIO_PIN_RESET)



/****************************************** ESP8266 函数声明 ***********************************************/
void                     ESP8266_Init                        ( void );
void                     ESP8266_Rst                         ( void );
bool                     ESP8266_Cmd                         ( char * cmd, char * reply1, char * reply2, u32 waittime );
bool                     ESP8266_AT_Test                     ( void );
bool                     ESP8266_Net_Mode_Choose             ( ENUM_Net_ModeTypeDef enumMode );
bool                     ESP8266_JoinAP                      ( char * pSSID, char * pPassWord );
bool                     ESP8266_BuildAP                     ( char * pSSID, char * pPassWord, ENUM_AP_PsdMode_TypeDef enunPsdMode );
bool                     ESP8266_Enable_MultipleId           ( FunctionalState enumEnUnvarnishTx );
bool                     ESP8266_Link_Server                 ( ENUM_NetPro_TypeDef enumE, char * ip, char * ComNum, ENUM_ID_NO_TypeDef id);
bool                     ESP8266_StartOrShutServer           ( FunctionalState enumMode, char * pPortNum, char * pTimeOver );
uint8_t                  ESP8266_Get_LinkStatus              ( void );
uint8_t                  ESP8266_Get_IdLinkStatus            ( void );
uint8_t                  ESP8266_Inquire_ApIp                ( char * pApIp, uint8_t ucArrayLength );
bool                     ESP8266_UnvarnishSend               ( void );
void                     ESP8266_ExitUnvarnishSend           ( void );
bool                     ESP8266_SendString                  ( FunctionalState enumEnUnvarnishTx, char * pStr, u32 ulStrLength, ENUM_ID_NO_TypeDef ucId );
bool					 ESP8266_SendBinary					 (uint8_t *pData, uint32_t ulDataLength, ENUM_ID_NO_TypeDef ucId);
char *                   ESP8266_ReceiveString               ( FunctionalState enumEnUnvarnishTx );
bool                     ESP8266_DHCP_CUR                    ( void );
/**
  * @brief  配置 ESP8266 USART 的 NVIC 中断
  * @param  无
  * @retval 无
  */
static void ESP8266_USART_NVIC_Configuration(void);
/*
 * 函数名：ESP8266_CIPAP
 * 描述  ：设置模块的 AP IP
 * 输入  ：pApIp，模块的 AP IP
 * 返回  : 1，设置成功
 *         0，设置失败
 * 调用  ：被外部调用
 */
uint8_t ESP8266_CIPAP ( char * pApIp );
#endif


