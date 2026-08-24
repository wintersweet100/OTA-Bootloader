#include "bsp_esp8266_test.h"
#include "bsp_esp8266.h"
#include <stdio.h>  
#include <string.h>  
#include "usart.h"
#include "ring_buffer.h"

#define LED_CMD_NUMBER   8
//char *ledCmd[ LED_CMD_NUMBER ] = { "LED_RED","LED_GREEN","LED_BLUE","LED_YELLOW","LED_PURPLE","LED_CYAN","LED_WHITE","LED_RGBOFF" };
             
volatile uint8_t ucTcpClosedFlag = 0;
extern ring_buffer rx_buf;
extern UART_HandleTypeDef huart_esp8266;
//extern UART_HandleTypeDef huart1;
extern uint8_t g_WiFi_Shell_Enable;


/**
  * @brief  ESP8266 ApTcpServer 配置测试函数
  * @param  无
  * @retval 无
  */
void ESP8266_ApTcpServer_ConfigTest(void)
{  
	printf( "\r\n正在配置 ESP8266 ......\r\n" );
	printf( "\r\n使能 ESP8266 ......\r\n" );
	macESP8266_CH_ENABLE();
	while( ! ESP8266_AT_Test() );

	while(! ESP8266_Cmd("ATE0", "OK", NULL, 500));//关闭回显

	printf( "\r\n正在配置工作模式为 AP ......\r\n" );
	while( ! ESP8266_Net_Mode_Choose ( AP ) );

	printf( "\r\n正在创建WiFi热点 ......\r\n" );
	while ( ! ESP8266_CIPAP ( macUser_ESP8266_TcpServer_IP ) ); //设置模块的 AP IP
	while ( ! ESP8266_BuildAP ( macUser_ESP8266_BulitApSsid, macUser_ESP8266_BulitApPwd, macUser_ESP8266_BulitApEcn ) );
	
	printf( "\r\n允许多连接 ......\r\n" );
	while( ! ESP8266_Enable_MultipleId ( ENABLE ) );
  
	printf( "\r\n开启服务器模式 ......\r\n" );
	while ( !	ESP8266_StartOrShutServer ( ENABLE, macUser_ESP8266_TcpServer_Port, macUser_ESP8266_TcpServer_OverTime ) );

  //ESP8266_Inquire_ApIp ( cStr, 20 );
	printf ( "\n本模块WIFI为 %s，密码开放\nAP IP 为：%s，开启的端口为：%s\n手机网络助手连接该 IP 和端口，最多可连接5个客户端\n",
           macUser_ESP8266_BulitApSsid, macUser_ESP8266_TcpServer_IP, macUser_ESP8266_TcpServer_Port );
	
	
	//g_WiFi_Shell_Enable = 1;//开启串口3转发，即esp8266端会回显接收到的数据

	printf("\r\nWaiting for client connection...\r\n");

}

void ESP8266_CheckConnectTest(void)
{
    char *pData;
    static uint8_t client_connected = 0; // 标记客户端是否已连接

    /* --- 1. 检测客户端连接事件 --- */
    if(strEsp8266_Fram_Record.InfBit.FramFinishFlag)
    {
        strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength] = '\0';
        
        // ? 检测到客户端连接
        if(strstr((char*)strEsp8266_Fram_Record.Data_RX_BUF, ",CONNECT"))
        {
            printf("\r\n[System] Client connected!\r\n");
            client_connected = 1;
            g_WiFi_Shell_Enable = 1; // ? 只有连接后才开启转发
        }
        
        // ? 检测到客户端断开
        if(strstr((char*)strEsp8266_Fram_Record.Data_RX_BUF, "CLOSED"))
        {
            printf("\r\n[System] Client disconnected!\r\n");
            client_connected = 0;
            ucTcpClosedFlag = 1;
            g_WiFi_Shell_Enable = 0; // 断开后关闭转发
        }
        
        // 处理接收到的数据
        pData = strstr((char*)strEsp8266_Fram_Record.Data_RX_BUF, ":");
        if(pData != NULL)
        {
            pData++; // 跳过冒号
            printf("%s", pData); 
        }
        
        // 清空标志
        memset(strEsp8266_Fram_Record.Data_RX_BUF, 0, sizeof(strEsp8266_Fram_Record.Data_RX_BUF));
        strEsp8266_Fram_Record.InfBit.FramLength = 0;
        strEsp8266_Fram_Record.InfBit.FramFinishFlag = 0;
    }

    /* --- 2. 只在客户端已连接时才转发串口1的数据 --- */
    if(strUSART_Fram_Record.InfBit.FramFinishFlag == 1 && client_connected)
    {
        ESP8266_SendString(DISABLE, (char *)strUSART_Fram_Record.Data_RX_BUF, 
                           strUSART_Fram_Record.InfBit.FramLength, Multiple_ID_0);

        memset(strUSART_Fram_Record.Data_RX_BUF, 0, sizeof(strUSART_Fram_Record.Data_RX_BUF));
        strUSART_Fram_Record.InfBit.FramLength = 0;
        strUSART_Fram_Record.InfBit.FramFinishFlag = 0;
    }
}

/**
  * @brief  ESP8266 AP模式下的数据互传测试函数
  * @note   实现功能：
  *         1. 串口助手发什么，ESP8266就转发给连接它的第一个客户端(ID 0)
  *         2. 网络助手发什么，ESP8266就原样打印到串口助手
  */
void ESP8266_CheckRecvDataTest(void)
{
  char *pData;

  /* --- 1. 处理串口调试助手发来的数据 (电脑 -> ESP8266 -> 网络调试助手) --- */
  if(strUSART_Fram_Record.InfBit.FramFinishFlag == 1)
  {
    /* 
       重要：AP模式Server下不支持透传，必须调用带 AT+CIPSEND 的发送函数 
       参数1：DISABLE 表示非透传模式
       参数2：要发送的数据地址
       参数3：数据长度
       参数4：目标客户端ID。手机连接AP后，第一个客户端ID通常是 Single_ID_0
    */
	//__HAL_UART_DISABLE_IT(&huart1, UART_IT_RXNE);

    ESP8266_SendString(DISABLE, (char *)strUSART_Fram_Record.Data_RX_BUF, 
                       strUSART_Fram_Record.InfBit.FramLength, Multiple_ID_0);

	memset(strUSART_Fram_Record.Data_RX_BUF,0,sizeof(strUSART_Fram_Record.Data_RX_BUF));
    // 发送完后清空标志
    strUSART_Fram_Record.InfBit.FramLength = 0;
    strUSART_Fram_Record.InfBit.FramFinishFlag = 0;

	//__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);//恢复接收中断
  }

  /* --- 2. 处理 ESP8266 接收到的数据 (网络调试助手 -> ESP8266 -> 串口) --- */
  if(strEsp8266_Fram_Record.InfBit.FramFinishFlag)
  {
    /* 
       ESP8266 在非透传模式下，收到的原始数据格式是 "+IPD,0,n:message"
       我们需要判断是否包含 +IPD
    */
	strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength] = '\0';

	pData = strstr((char*)strEsp8266_Fram_Record.Data_RX_BUF,":");

     if(pData != NULL)
     {
         pData++; // 指向冒号后面的第一个字符
         // 把收到的内容打印到调试串口（电脑）
         printf("%s", pData); 
     }
	 else
     {
        // 如果是一些系统提示信息（如 0,CONNECT），也直接打印出来方便调试
        //printf("%s", strEsp8266_Fram_Record.Data_RX_BUF);
     }
    // 处理完后清空标志
	memset(strEsp8266_Fram_Record.Data_RX_BUF,0,sizeof(strEsp8266_Fram_Record.Data_RX_BUF));
    strEsp8266_Fram_Record.InfBit.FramLength = 0;
    strEsp8266_Fram_Record.InfBit.FramFinishFlag = 0;
  }
}


//这是在原版TCP-Client透传测试程序上改造的可以与shell进行数据交互的一版
//因为目前考虑将ESP-01s作为服务端更好一点，便舍弃
/**
  * @brief  ESP8266 StaTcpClient Unvarnish 配置测试函数
  * @param  无
  * @retval 无
  */
void ESP8266_StaTcpClient_Unvarnish_ConfigTest(void)
{
  printf( "\r\nConfiguring ESP8266 ......\r\n" );
  printf( "\r\nEnable ESP8266 ......\r\n" );
	macESP8266_CH_ENABLE();
	while( ! ESP8266_AT_Test() );
	printf( "\r\nStart DHCP ......\r\n" );
  while( ! ESP8266_DHCP_CUR () );  
  printf( "\r\nConfiguring the working mode:sSTA ......\r\n" );
	while( ! ESP8266_Net_Mode_Choose ( STA ) );

  printf( "\r\nConnecting WiFi ......\r\n" );
  while( ! ESP8266_JoinAP ( macUser_ESP8266_ApSsid, macUser_ESP8266_ApPwd ) );	
	
  printf( "\r\nProhibit multiple connections ......\r\n" );
	while( ! ESP8266_Enable_MultipleId ( DISABLE ) );
	
  printf( "\r\nnConnecting Server ......\r\n" );
	while( !	ESP8266_Link_Server ( enumTCP, macUser_ESP8266_TcpServer_IP, macUser_ESP8266_TcpServer_Port, Single_ID_0 ) );
	
  printf( "\r\nEnter transparent transmission mode ......\r\n" );
	while( ! ESP8266_UnvarnishSend () );
	
	printf( "\r\nnConfiguring ESP8266 end\r\n" );
	printf ( "\r\nStart transparent transmission......\r\n" );
  
	
	g_WiFi_Shell_Enable = 1;//开启串口3转发，即esp8266端会回显接收到的数据

	printf("\r\nWiFi Shell Ready!\r\n");
}

#if 0 
/**
  * @brief  适配 Shell 环境的 ESP8266 数据转发与掉线检查
  */
void ESP8266_CheckRecvDataTest(void)
{
    uint8_t ucStatus;
    uint8_t traCh;

    /* 1. 从 Shell (串口1) 的环形队列取出数据，转发给 ESP8266 (串口3) */
    // ring_buffer_read 成功返回 0，只要有数据就一直读
    while (ring_buffer_read(&traCh, &rx_buf) == 0)
    {
        // 直接使用 HAL 库原生发送函数
        // 参数：串口3句柄, 数据地址, 长度1, 超时10ms
        HAL_UART_Transmit(&huart_esp8266, &traCh, 1, 10);
    }

    /* 2. 处理 ESP8266 (串口3) 回传的数据 -> 显示到 Shell (串口1) */
    if (strEsp8266_Fram_Record.InfBit.FramFinishFlag == 1)
    {                                                      
        // 这里的 putchar 已经重定向到了串口1
        for(uint16_t i = 0; i < strEsp8266_Fram_Record.InfBit.FramLength; i++)
        {
            putchar(strEsp8266_Fram_Record.Data_RX_BUF[i]);
        }
        // 清空串口3接收缓存
        strEsp8266_Fram_Record.InfBit.FramLength = 0;
        strEsp8266_Fram_Record.InfBit.FramFinishFlag = 0;
    }

    /* 3. 检测掉线并自动重连 */
    if (ucTcpClosedFlag)
    {
        ESP8266_ExitUnvarnishSend(); // 退出透传
        
        do {
            ucStatus = ESP8266_Get_LinkStatus();
        } while ( !ucStatus ); // 等待获取到状态

        if (ucStatus == 4) // 状态4表示确定断开
        {
            printf("\r\n[System] Connection Lost! Reconnecting...\r\n");
            while (!ESP8266_JoinAP(macUser_ESP8266_ApSsid, macUser_ESP8266_ApPwd));
            while (!ESP8266_Link_Server(enumTCP, macUser_ESP8266_TcpServer_IP, macUser_ESP8266_TcpServer_Port, Single_ID_0));
            printf("\r\n[System] Reconnect OK.\r\n");
        }
        
        while (!ESP8266_UnvarnishSend()); // 重新开启透传
        ucTcpClosedFlag = 0;
    }
}
#endif

#if 0
	//例程提供的初始化原版、及测试函数原版
void ESP8266_StaTcpClient_Unvarnish_ConfigTest(void)
{
  printf( "\r\n正在配置 ESP8266 ......\r\n" );
  printf( "\r\n使能 ESP8266 ......\r\n" );
	macESP8266_CH_ENABLE();
	while( ! ESP8266_AT_Test() );
	printf( "\r\n开启DHCP ......\r\n" );
  while( ! ESP8266_DHCP_CUR () );  
  printf( "\r\n正在配置工作模式 STA ......\r\n" );
	while( ! ESP8266_Net_Mode_Choose ( STA ) );

  printf( "\r\n正在连接 WiFi ......\r\n" );
  while( ! ESP8266_JoinAP ( macUser_ESP8266_ApSsid, macUser_ESP8266_ApPwd ) );	
	
  printf( "\r\n禁止多连接 ......\r\n" );
	while( ! ESP8266_Enable_MultipleId ( DISABLE ) );
	
  printf( "\r\n正在连接 Server ......\r\n" );
	while( !	ESP8266_Link_Server ( enumTCP, macUser_ESP8266_TcpServer_IP, macUser_ESP8266_TcpServer_Port, Single_ID_0 ) );
	
  printf( "\r\n进入透传发送模式 ......\r\n" );
	while( ! ESP8266_UnvarnishSend () );
	
	printf( "\r\n配置 ESP8266 完毕\r\n" );
	printf ( "\r\n开始透传......\r\n" );
  
}


/**
  * @brief  ESP8266 检查是否接收到了数据，检查连接和掉线重连
  * @param  无
  * @retval 无
  */
void ESP8266_CheckRecvDataTest(void)
{
  uint8_t ucStatus;
  uint16_t i;
  
  /* 如果接收到了串口调试助手的数据 */
  if(strUSART_Fram_Record.InfBit.FramFinishFlag == 1)
  {
    for(i = 0;i < strUSART_Fram_Record.InfBit.FramLength; i++)
    {
       USART_SendData( macESP8266_USARTx ,strUSART_Fram_Record.Data_RX_BUF[i]); //转发给ESP82636
       while(USART_GetFlagStatus(macESP8266_USARTx,USART_FLAG_TC)==RESET){}      //等待发送完成
    }
    strUSART_Fram_Record .InfBit .FramLength = 0;                                //接收数据长度置零
    strUSART_Fram_Record .InfBit .FramFinishFlag = 0;                            //接收标志置零
    Get_ESP82666_Cmd(strUSART_Fram_Record .Data_RX_BUF);                         //检查一下是不是点灯命令
  }
  
  /* 如果接收到了ESP8266的数据 */
  if(strEsp8266_Fram_Record.InfBit.FramFinishFlag)
  {                                                      
    for(i = 0;i < strEsp8266_Fram_Record .InfBit .FramLength; i++)               
    {
       USART_SendData( DEBUG_USARTx ,strEsp8266_Fram_Record .Data_RX_BUF[i]);    //转发给ESP8266
       while(USART_GetFlagStatus(DEBUG_USARTx,USART_FLAG_TC)==RESET){}
    }
     strEsp8266_Fram_Record .InfBit .FramLength = 0;                             //接收数据长度置零
     strEsp8266_Fram_Record.InfBit.FramFinishFlag = 0;                           //接收标志置零
     Get_ESP82666_Cmd(strEsp8266_Fram_Record .Data_RX_BUF);                      //检查一下是不是点灯命令
  }
  
  if ( ucTcpClosedFlag )                                             //检测是否失去连接
  {
    ESP8266_ExitUnvarnishSend ();                                    //退出透传模式
    
    do ucStatus = ESP8266_Get_LinkStatus ();                         //获取连接状态
    while ( ! ucStatus );
    
    if ( ucStatus == 4 )                                             //确认失去连接后重连
    {
      printf ( "\r\n正在重连热点和服务器 ......\r\n" );
      
      while ( ! ESP8266_JoinAP ( macUser_ESP8266_ApSsid, macUser_ESP8266_ApPwd ) );
      
      while ( !	ESP8266_Link_Server ( enumTCP, macUser_ESP8266_TcpServer_IP, macUser_ESP8266_TcpServer_Port, Single_ID_0 ) );
      
      printf ( "\r\n重连热点和服务器成功\r\n" );

    }
    
    while ( ! ESP8266_UnvarnishSend () );		
    
  }
}
#endif




