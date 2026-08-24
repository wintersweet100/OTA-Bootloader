#include "bsp_esp8266.h"
#include <stdio.h>
#include <string.h>  
#include <stdarg.h>
#include "core_delay.h"

static void                   ESP8266_GPIO_Config                 ( void );
static void                   ESP8266_USART_Config                ( void );
static void                   ESP8266_USART_NVIC_Configuration    ( void );


struct  STRUCT_USARTx_Fram strEsp8266_Fram_Record = { 0 };
struct  STRUCT_USARTx_Fram strUSART_Fram_Record = { 0 };


/**
  * @brief  ESP8266初始化函数
  * @param  无
  * @retval 无
  */
void ESP8266_Init ( void )
{
	ESP8266_GPIO_Config (); 
	
	ESP8266_USART_Config (); 
	
	macESP8266_RST_HIGH_LEVEL();

	macESP8266_CH_DISABLE();
}

/**
  * @brief  初始化ESP8266用到的GPIO引脚
  * @param  无	
  * @retval 无
  */
static void ESP8266_GPIO_Config ( void )
{
	/*定义一个GPIO_InitTypeDef类型的结构体*/
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 配置 CH_PD 引脚*/
	__HAL_RCC_GPIOG_CLK_ENABLE();
											   
	GPIO_InitStructure.Pin = macESP8266_CH_PD_PIN;	

	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;   
   
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH; 
    // HAL库需要设置上下拉
    GPIO_InitStructure.Pull = GPIO_NOPULL;	

	HAL_GPIO_Init ( macESP8266_CH_PD_PORT, & GPIO_InitStructure );	 

	
	/* 配置 RST 引脚*/									   
	GPIO_InitStructure.Pin = macESP8266_RST_PIN;	

	HAL_GPIO_Init ( macESP8266_RST_PORT, & GPIO_InitStructure );	 


}


/**
  * @brief  初始化ESP8266用到的 USART
  * @param  无
  * @retval 无
  */

// 在文件开头定义句柄
UART_HandleTypeDef huart_esp8266;

static void ESP8266_USART_Config(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 1. 使能时钟
    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // 2. 配置 TX 引脚 (PB10)
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;    // 必须是复用推挽输出
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 3. 配置 RX 引脚 (PB11)
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;    // F1系列RX通常设为浮空输入
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 4. 配置 UART 参数
    huart_esp8266.Instance        = macESP8266_USARTx; // USART3
    huart_esp8266.Init.BaudRate   = 115200;
    huart_esp8266.Init.WordLength = UART_WORDLENGTH_8B;
    huart_esp8266.Init.StopBits   = UART_STOPBITS_1;
    huart_esp8266.Init.Parity     = UART_PARITY_NONE;
    huart_esp8266.Init.Mode       = UART_MODE_TX_RX;
    huart_esp8266.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    
    // 5. 执行初始化
    HAL_UART_Init(&huart_esp8266);

    // 6. 开启中断
    __HAL_UART_ENABLE_IT(&huart_esp8266, UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&huart_esp8266, UART_IT_IDLE);

    // 7. 配置中断优先级并使能
    ESP8266_USART_NVIC_Configuration();
}

#if 0
static void ESP8266_USART_Config ( void )
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	
	/* config USART clock */
	macESP8266_USART_APBxClock_FUN ( macESP8266_USART_CLK, ENABLE );
	macESP8266_USART_GPIO_APBxClock_FUN ( macESP8266_USART_GPIO_CLK, ENABLE );
	
	/* USART GPIO config */
	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitStructure.Pin =  macESP8266_USART_TX_PIN;
	GPIO_InitStructure.Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_Init(macESP8266_USART_TX_PORT, &GPIO_InitStructure);  
  
	/* Configure USART Rx as input floating */
	GPIO_InitStructure.Pin = macESP8266_USART_RX_PIN;
	GPIO_InitStructure.Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(macESP8266_USART_RX_PORT, &GPIO_InitStructure);
	
	/* USART1 mode config */
	USART_InitStructure.USART_BaudRate = macESP8266_USART_BAUD_RATE;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(macESP8266_USARTx, &USART_InitStructure);
	
	
	/* 中断配置 */
	USART_ITConfig ( macESP8266_USARTx, USART_IT_RXNE, ENABLE ); //使能串口接收中断 
	USART_ITConfig ( macESP8266_USARTx, USART_IT_IDLE, ENABLE ); //使能串口总线空闲中断 	

	ESP8266_USART_NVIC_Configuration ();
	
	
	USART_Cmd(macESP8266_USARTx, ENABLE);
	
	
}
#endif


/**
  * @brief  配置 ESP8266 USART 的 NVIC 中断
  * @param  无
  * @retval 无
  */
static void ESP8266_USART_NVIC_Configuration(void) {
    // 设置优先级分组（通常在 main 的 HAL_Init 已设置，这里可省）
    HAL_NVIC_SetPriority(macESP8266_USART_IRQ, 1, 0); // 抢占优先级0，子优先级0
    HAL_NVIC_EnableIRQ(macESP8266_USART_IRQ);         // 使能中断通道
}

#if 0
static void ESP8266_USART_NVIC_Configuration ( void )
{
	NVIC_InitTypeDef NVIC_InitStructure; 
	
	
	/* Configure the NVIC Preemption Priority Bits */  
	NVIC_PriorityGroupConfig ( macNVIC_PriorityGroup_x );

	/* Enable the USART2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = macESP8266_USART_IRQ;	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}
#endif

void USART_printf(USART_TypeDef *USARTx, char *Data, ...)
{
    const char *s;
    int d;
    char buf[16];
    va_list ap;
    va_start(ap, Data);

    while (*Data != 0)
    {
        if (*Data == '%')
        {
            switch (*++Data)
            {
                case 's': // 发送字符串
                    s = va_arg(ap, const char *);
                    // 找到字符串长度
                    HAL_UART_Transmit(&huart_esp8266, (uint8_t *)s, strlen(s), 100);
                    Data++;
                    break;

                case 'd': // 发送整数
                    d = va_arg(ap, int);
                    itoa(d, buf, 10);
                    HAL_UART_Transmit(&huart_esp8266, (uint8_t *)buf, strlen(buf), 100);
                    Data++;
                    break;

                default:
                    Data++;
                    break;
            }
        }
        else
        {
            // 发送单个普通字符
            HAL_UART_Transmit(&huart_esp8266, (uint8_t *)Data, 1, 100);
            Data++;
        }
    }
    va_end(ap);
}
/*
 * 函数名：itoa
 * 描述  ：将整形数据转换成字符串
 * 输入  ：-radix =10 表示10进制，其他结果为0
 *         -value 要转换的整形数
 *         -buf 转换后的字符串
 *         -radix = 10
 * 输出  ：无
 * 返回  ：无
 * 调用  ：被USART2_printf()调用
 */
static char * itoa( int value, char *string, int radix )
{
	int     i, d;
	int     flag = 0;
	char    *ptr = string;

	/* This implementation only works for decimal numbers. */
	if (radix != 10)
	{
		*ptr = 0;
		return string;
	}

	if (!value)
	{
		*ptr++ = 0x30;
		*ptr = 0;
		return string;
	}

	/* if this is a negative value insert the minus sign. */
	if (value < 0)
	{
		*ptr++ = '-';

		/* Make the value positive. */
		value *= -1;
		
	}

	for (i = 10000; i > 0; i /= 10)
	{
		d = value / i;

		if (d || flag)
		{
			*ptr++ = (char)(d + 0x30);
			value -= (d * i);
			flag = 1;
		}
	}

	/* Null terminate the string. */
	*ptr = 0;

	return string;

} /* NCL_Itoa */


/*
 * 函数名：ESP8266_Rst
 * 描述  ：重启WF-ESP8266模块
 * 输入  ：无
 * 返回  : 无
 * 调用  ：被 ESP8266_AT_Test 调用
 */
void ESP8266_Rst ( void )
{
	#if 0
	 ESP8266_Cmd ( "AT+RST", "OK", "ready", 2500 );   	
	
	#else
	 macESP8266_RST_LOW_LEVEL();
	 Delay_ms ( 500 ); 
	 macESP8266_RST_HIGH_LEVEL();
	#endif

}


/*
 * 函数名：ESP8266_Cmd
 * 描述  ：对WF-ESP8266模块发送AT指令
 * 输入  ：cmd，待发送的指令
 *         reply1，reply2，期待的响应，为NULL表不需响应，两者为或逻辑关系
 *         waittime，等待响应的时间
 * 返回  : 1，指令发送成功
 *         0，指令发送失败
 * 调用  ：被外部调用
 */
bool ESP8266_Cmd ( char * cmd, char * reply1, char * reply2, u32 waittime )
{    
	strEsp8266_Fram_Record .InfBit .FramLength = 0;               //从新开始接收新的数据包

	macESP8266_Usart ( "%s\r\n", cmd );

	if ( ( reply1 == 0 ) && ( reply2 == 0 ) )                      //不需要接收数据
		return true;
	
	Delay_ms ( waittime );                 //延时

	//printf("接收到的原始数据: [%s]\r\n", strEsp8266_Fram_Record.Data_RX_BUF);
	printf("Received raw data:[%s]\r\n", strEsp8266_Fram_Record.Data_RX_BUF);

	strEsp8266_Fram_Record .Data_RX_BUF [ strEsp8266_Fram_Record .InfBit .FramLength ]  = '\0';

	macPC_Usart ( "%s", strEsp8266_Fram_Record .Data_RX_BUF );
	strEsp8266_Fram_Record .InfBit .FramLength = 0;                             //清除接收标志
	strEsp8266_Fram_Record.InfBit.FramFinishFlag = 0;                             
	if ( ( reply1 != 0 ) && ( reply2 != 0 ) )
		return ( ( bool ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, reply1 ) || 
						 ( bool ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, reply2 ) ); 
 	
	else if ( reply1 != 0 )
		return ( ( bool ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, reply1 ) );
	
	else
		return ( ( bool ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, reply2 ) );
	
}


/*
 * 函数名：ESP8266_AT_Test
 * 描述  ：对WF-ESP8266模块进行AT测试启动
 * 输入  ：无
 * 返回  : 无
 * 调用  ：被外部调用
 */
//void ESP8266_AT_Test ( void )
//{
//	macESP8266_RST_HIGH_LEVEL();
//	
//	Delay_ms ( 1000 ); 
//	
//	while ( ! ESP8266_Cmd ( "AT", "OK", NULL, 500 ) ) ESP8266_Rst ();  	

//}
bool ESP8266_AT_Test ( void )
{
	char count=0;
	
	macESP8266_RST_HIGH_LEVEL();	
	printf("\r\nAT test.....\r\n");
	Delay_ms ( 2000 );
	while ( count < 10 )
	{
		printf("\r\nNumber of AT tests:%d......\r\n", count);
		if( ESP8266_Cmd ( "AT", "OK", NULL, 500 ) )
    {
      printf("\r\nThe AT test started successfully %d......\r\n", count);
      return 1;
    }
		ESP8266_Rst();
		++ count;
	}
  return 0;
}


bool ESP8266_DHCP_CUR ( )
{
	char cCmd [40];

	sprintf ( cCmd, "AT+CWDHCP_CUR=1,1");
	
	return ESP8266_Cmd ( cCmd, "OK", NULL, 500 );
	
}


/*
 * 函数名：ESP8266_Net_Mode_Choose
 * 描述  ：选择WF-ESP8266模块的工作模式
 * 输入  ：enumMode，工作模式
 * 返回  : 1，选择成功
 *         0，选择失败
 * 调用  ：被外部调用
 */
bool ESP8266_Net_Mode_Choose ( ENUM_Net_ModeTypeDef enumMode )
{
	switch ( enumMode )
	{
		case STA:
			return ESP8266_Cmd ( "AT+CWMODE=1", "OK", "no change", 2500 ); 
		
	   case AP:
		  return ESP8266_Cmd ( "AT+CWMODE=2", "OK", "no change", 2500 ); 
		
		case STA_AP:
		  return ESP8266_Cmd ( "AT+CWMODE=3", "OK", "no change", 2500 ); 
		
	  default:
		  return false;
  }
	
}


/*
 * 函数名：ESP8266_JoinAP
 * 描述  ：WF-ESP8266模块连接外部WiFi
 * 输入  ：pSSID，WiFi名称字符串
 *       ：pPassWord，WiFi密码字符串
 * 返回  : 1，连接成功
 *         0，连接失败
 * 调用  ：被外部调用
 */
bool ESP8266_JoinAP ( char * pSSID, char * pPassWord )
{
	char cCmd [120];

	sprintf ( cCmd, "AT+CWJAP=\"%s\",\"%s\"", pSSID, pPassWord );
	
	return ESP8266_Cmd ( cCmd, "OK", NULL, 5000 );
	
}


/*
 * 函数名：ESP8266_BuildAP
 * 描述  ：WF-ESP8266模块创建WiFi热点
 * 输入  ：pSSID，WiFi名称字符串
 *       ：pPassWord，WiFi密码字符串
 *       ：enunPsdMode，WiFi加密方式代号字符串
 * 返回  : 1，创建成功
 *         0，创建失败
 * 调用  ：被外部调用
 */
bool ESP8266_BuildAP ( char * pSSID, char * pPassWord, ENUM_AP_PsdMode_TypeDef enunPsdMode )
{
	char cCmd [120];

	sprintf ( cCmd, "AT+CWSAP=\"%s\",\"%s\",1,%d", pSSID, pPassWord, enunPsdMode );
	
	return ESP8266_Cmd ( cCmd, "OK", 0, 1000 );
	
}


/*
 * 函数名：ESP8266_Enable_MultipleId
 * 描述  ：WF-ESP8266模块启动多连接
 * 输入  ：enumEnUnvarnishTx，配置是否多连接
 * 返回  : 1，配置成功
 *         0，配置失败
 * 调用  ：被外部调用
 */
bool ESP8266_Enable_MultipleId ( FunctionalState enumEnUnvarnishTx )
{
	char cStr [20];
	
	sprintf ( cStr, "AT+CIPMUX=%d", ( enumEnUnvarnishTx ? 1 : 0 ) );
	
	return ESP8266_Cmd ( cStr, "OK", 0, 500 );
	
}


/*
 * 函数名：ESP8266_Link_Server
 * 描述  ：WF-ESP8266模块连接外部服务器
 * 输入  ：enumE，网络协议
 *       ：ip，服务器IP字符串
 *       ：ComNum，服务器端口字符串
 *       ：id，模块连接服务器的ID
 * 返回  : 1，连接成功
 *         0，连接失败
 * 调用  ：被外部调用
 */
bool ESP8266_Link_Server ( ENUM_NetPro_TypeDef enumE, char * ip, char * ComNum, ENUM_ID_NO_TypeDef id)
{
	char cStr [100] = { 0 }, cCmd [120];

  switch (  enumE )
  {
		case enumTCP:
		  sprintf ( cStr, "\"%s\",\"%s\",%s", "TCP", ip, ComNum );
		  break;
		
		case enumUDP:
		  sprintf ( cStr, "\"%s\",\"%s\",%s", "UDP", ip, ComNum );
		  break;
		
		default:
			break;
  }

  if ( id < 5 )
    sprintf ( cCmd, "AT+CIPSTART=%d,%s", id, cStr);

  else
	  sprintf ( cCmd, "AT+CIPSTART=%s", cStr );

	return ESP8266_Cmd ( cCmd, "OK", "ALREAY CONNECT", 4000 );
	
}


/*
 * 函数名：ESP8266_StartOrShutServer
 * 描述  ：WF-ESP8266模块开启或关闭服务器模式
 * 输入  ：enumMode，开启/关闭
 *       ：pPortNum，服务器端口号字符串
 *       ：pTimeOver，服务器超时时间字符串，单位：秒
 * 返回  : 1，操作成功
 *         0，操作失败
 * 调用  ：被外部调用
 */
bool ESP8266_StartOrShutServer ( FunctionalState enumMode, char * pPortNum, char * pTimeOver )
{
	char cCmd1 [120], cCmd2 [120];

	if ( enumMode )
	{
		sprintf ( cCmd1, "AT+CIPSERVER=%d,%s", 1, pPortNum );
		
		sprintf ( cCmd2, "AT+CIPSTO=%s", pTimeOver );

		return ( ESP8266_Cmd ( cCmd1, "OK", 0, 500 ) &&
						 ESP8266_Cmd ( cCmd2, "OK", 0, 500 ) );
	}
	
	else
	{
		sprintf ( cCmd1, "AT+CIPSERVER=%d,%s", 0, pPortNum );

		return ESP8266_Cmd ( cCmd1, "OK", 0, 500 );
	}
	
}


/*
 * 函数名：ESP8266_Get_LinkStatus
 * 描述  ：获取 WF-ESP8266 的连接状态，较适合单端口时使用
 * 输入  ：无
 * 返回  : 2，获得ip
 *         3，建立连接
 *         3，失去连接
 *         0，获取状态失败
 * 调用  ：被外部调用
 */
uint8_t ESP8266_Get_LinkStatus ( void )
{
	if ( ESP8266_Cmd ( "AT+CIPSTATUS", "OK", 0, 500 ) )
	{
		if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "STATUS:2\r\n" ) )
			return 2;
		
		else if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "STATUS:3\r\n" ) )
			return 3;
		
		else if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "STATUS:4\r\n" ) )
			return 4;		

	}
	
	return 0;
	
}


/*
 * 函数名：ESP8266_Get_IdLinkStatus
 * 描述  ：获取 WF-ESP8266 的端口（Id）连接状态，较适合多端口时使用
 * 输入  ：无
 * 返回  : 端口（Id）的连接状态，低5位为有效位，分别对应Id5~0，某位若置1表该Id建立了连接，若被清0表该Id未建立连接
 * 调用  ：被外部调用
 */
uint8_t ESP8266_Get_IdLinkStatus ( void )
{
	uint8_t ucIdLinkStatus = 0x00;
	
	
	if ( ESP8266_Cmd ( "AT+CIPSTATUS", "OK", 0, 500 ) )
	{
		if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "+CIPSTATUS:0," ) )
			ucIdLinkStatus |= 0x01;
		else 
			ucIdLinkStatus &= ~ 0x01;
		
		if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "+CIPSTATUS:1," ) )
			ucIdLinkStatus |= 0x02;
		else 
			ucIdLinkStatus &= ~ 0x02;
		
		if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "+CIPSTATUS:2," ) )
			ucIdLinkStatus |= 0x04;
		else 
			ucIdLinkStatus &= ~ 0x04;
		
		if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "+CIPSTATUS:3," ) )
			ucIdLinkStatus |= 0x08;
		else 
			ucIdLinkStatus &= ~ 0x08;
		
		if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "+CIPSTATUS:4," ) )
			ucIdLinkStatus |= 0x10;
		else 
			ucIdLinkStatus &= ~ 0x10;	

	}
	
	return ucIdLinkStatus;
	
}


/*
 * 函数名：ESP8266_Inquire_ApIp
 * 描述  ：获取 F-ESP8266 的 AP IP
 * 输入  ：pApIp，存放 AP IP 的数组的首地址
 *         ucArrayLength，存放 AP IP 的数组的长度
 * 返回  : 0，获取失败
 *         1，获取成功
 * 调用  ：被外部调用
 */
uint8_t ESP8266_Inquire_ApIp ( char * pApIp, uint8_t ucArrayLength )
{
	char uc;
	
	char * pCh;
	
	
  ESP8266_Cmd ( "AT+CIFSR", "OK", 0, 500 );
	
	pCh = strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "APIP,\"" );
	
	if ( pCh )
		pCh += 6;
	
	else
		return 0;
	
	for ( uc = 0; uc < ucArrayLength; uc ++ )
	{
		pApIp [ uc ] = * ( pCh + uc);
		
		if ( pApIp [ uc ] == '\"' )
		{
			pApIp [ uc ] = '\0';
			break;
		}
		
	}
	
	return 1;
	
}


/*
 * 函数名：ESP8266_UnvarnishSend
 * 描述  ：配置WF-ESP8266模块进入透传发送
 * 输入  ：无
 * 返回  : 1，配置成功
 *         0，配置失败
 * 调用  ：被外部调用
 */
bool ESP8266_UnvarnishSend ( void )
{
	if ( ! ESP8266_Cmd ( "AT+CIPMODE=1", "OK", 0, 500 ) )
		return false;
	
	return 
	  ESP8266_Cmd ( "AT+CIPSEND", "OK", ">", 500 );
	
}


/*
 * 函数名：ESP8266_ExitUnvarnishSend
 * 描述  ：配置WF-ESP8266模块退出透传模式
 * 输入  ：无
 * 返回  : 无
 * 调用  ：被外部调用
 */
void ESP8266_ExitUnvarnishSend ( void )
{
	Delay_ms ( 1000 );
	
	macESP8266_Usart ( "+++" );
	
	Delay_ms( 500 ); 
	
}


/*
 * 函数名：ESP8266_SendString
 * 描述  ：WF-ESP8266模块发送字符串
 * 输入  ：enumEnUnvarnishTx，声明是否已使能了透传模式
 *       ：pStr，要发送的字符串
 *       ：ulStrLength，要发送的字符串的字节数
 *       ：ucId，哪个ID发送的字符串
 * 返回  : 1，发送成功
 *         0，发送失败
 * 调用  ：被外部调用
 */
bool ESP8266_SendString ( FunctionalState enumEnUnvarnishTx, char * pStr, u32 ulStrLength, ENUM_ID_NO_TypeDef ucId )
{
	char cStr [20];
	bool bRet = false;
	
		
	if ( enumEnUnvarnishTx ){
		macESP8266_Usart ( "%s", pStr );
		
		bRet = true;		
	}

	else{
		if ( ucId < 5 )
			sprintf ( cStr, "AT+CIPSEND=%d,%d", ucId, ulStrLength );

		else
			sprintf ( cStr, "AT+CIPSEND=%d", ulStrLength );

		strEsp8266_Fram_Record.InfBit.FramLength = 0;
		strEsp8266_Fram_Record.InfBit.FramFinishFlag = 0;
		memset(strEsp8266_Fram_Record.Data_RX_BUF, 0, RX_BUF_MAX_LEN);  //关键:：清空缓冲区！
	
        // 发送 AT 指令
        macESP8266_Usart("%s\r\n", cStr);
        
        // 等待 '>' 提示符（表示可以发送数据了）
		uint32_t timeout = 500;//最大500ms超时时间
		while(timeout > 0 && strstr((char*)strEsp8266_Fram_Record.Data_RX_BUF, ">") == NULL){
			Delay_ms(10);			
			timeout -= 10;
		}
		if(timeout <= 0 ){
			printf("[Esp8266] Timeout for waiting '>'\r\n");
			return false;
		}			
        
        // 第2步：发送真实数据（不要用 ESP8266_Cmd，不要自动加 \r\n）
        strEsp8266_Fram_Record.InfBit.FramLength = 0;
        strEsp8266_Fram_Record.InfBit.FramFinishFlag = 0;
		memset(strEsp8266_Fram_Record.Data_RX_BUF, 0, RX_BUF_MAX_LEN);  //关键:：清空缓冲区！        

        //  直接发送原始数据，不加任何额外字符
        HAL_UART_Transmit(&huart_esp8266, (uint8_t*)pStr, ulStrLength, 500);
        
        // 等待 SEND OK
        timeout = 1000;  // 1000ms 超时
        while (timeout > 0 && strstr((char*)strEsp8266_Fram_Record.Data_RX_BUF, "SEND OK") == NULL) {
            Delay_ms(10);
            timeout -= 10;
        }
        if (timeout <= 0) {
            printf("[ESP8266] Timeout waiting for 'SEND OK'\r\n");
            return false;
        }
        
        bRet = true;
  }
	
	return bRet;
}

// 新增：专门用于发送二进制数据的函数
bool ESP8266_SendBinary(uint8_t *pData, uint32_t ulDataLength, ENUM_ID_NO_TypeDef ucId)
{
    char cStr[20];
    
    // 第1步：清空缓冲区
    strEsp8266_Fram_Record.InfBit.FramLength = 0;
    strEsp8266_Fram_Record.InfBit.FramFinishFlag = 0;
    memset(strEsp8266_Fram_Record.Data_RX_BUF, 0, RX_BUF_MAX_LEN);
    
    // 第2步：发送 AT+CIPSEND 指令
    if (ucId < 5){
        sprintf(cStr, "AT+CIPSEND=%d,%d", ucId, ulDataLength);
	}
    else{
        sprintf(cStr, "AT+CIPSEND=%d", ulDataLength);
	}
    
    macESP8266_Usart("%s\r\n", cStr);
    //printf("[ESP8266] Sent: %s\r\n", cStr);
    
    // 第3步：等待 '>' 提示符
//    uint32_t timeout = 500;
//    while(timeout > 0 && strstr((char*)strEsp8266_Fram_Record.Data_RX_BUF, ">") == NULL) {
//        Delay_ms(10);
//        timeout -= 10;
//    }
//    
//    if(timeout <= 0) {
//        printf("[ESP8266] Timeout waiting for '>'\r\n");
//        return false;
//    }
    
    //printf("[ESP8266] Received '>', sending %d bytes of binary data\r\n", ulDataLength);
    
    // 第4步：清空缓冲区，准备接收 SEND OK
    strEsp8266_Fram_Record.InfBit.FramLength = 0;
    strEsp8266_Fram_Record.InfBit.FramFinishFlag = 0;
    memset(strEsp8266_Fram_Record.Data_RX_BUF, 0, RX_BUF_MAX_LEN);
    
    // 第5步：发送二进制数据（关键：直接发送，不做任何处理）
    HAL_UART_Transmit(&huart_esp8266, pData, ulDataLength, 1000);
    //printf("[ESP8266] Binary data sent\r\n");
    
    // 第6步：等待 SEND OK
//    uint32_t timeout = 100;
//    while (timeout > 0 && strstr((char*)strEsp8266_Fram_Record.Data_RX_BUF, "SEND OK") == NULL) {
//        Delay_ms(10);
//        timeout -= 10;
//    }
//    if (timeout <= 0) {
//        printf("[ESP8266] Timeout waiting for 'SEND OK'\r\n");
//        return false;
//    }
    
    //printf("[ESP8266] SEND OK received\r\n");
    return true;
}

/*
 * 函数名：ESP8266_ReceiveString
 * 描述  ：WF-ESP8266模块接收字符串
 * 输入  ：enumEnUnvarnishTx，声明是否已使能了透传模式
 * 返回  : 接收到的字符串首地址
 * 调用  ：被外部调用
 */
char * ESP8266_ReceiveString ( FunctionalState enumEnUnvarnishTx )
{
	char * pRecStr = 0;
	
	
	strEsp8266_Fram_Record .InfBit .FramLength = 0;
	strEsp8266_Fram_Record .InfBit .FramFinishFlag = 0;
	
	while ( ! strEsp8266_Fram_Record .InfBit .FramFinishFlag );
	strEsp8266_Fram_Record .Data_RX_BUF [ strEsp8266_Fram_Record .InfBit .FramLength ] = '\0';
	
	if ( enumEnUnvarnishTx )
		pRecStr = strEsp8266_Fram_Record .Data_RX_BUF;
	
	else 
	{
		if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "+IPD" ) )
			pRecStr = strEsp8266_Fram_Record .Data_RX_BUF;

	}

	return pRecStr;
	
}

/*
 * 函数名：ESP8266_CIPAP
 * 描述  ：设置模块的 AP IP
 * 输入  ：pApIp，模块的 AP IP
 * 返回  : 1，设置成功
 *         0，设置失败
 * 调用  ：被外部调用
 */
uint8_t ESP8266_CIPAP ( char * pApIp )
{
	char cCmd [ 30 ];
		
	
	sprintf ( cCmd, "AT+CIPAP=\"%s\"", pApIp );
	
  if ( ESP8266_Cmd ( cCmd, "OK", 0, 5000 ) )
		return 1;
 
	else 
		return 0;
	
}

