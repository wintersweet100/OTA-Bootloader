#include "command.h"
#include "usart.h"
#include "string.h"
#include "ring_buffer.h"
#include "bsp_esp8266.h"
#include "bsp_esp8266_test.h"
#include "core_delay.h"
#include "w5500.h"
#include "W5500_conf.h"
#include "socket.h"
#include "utility.h"
#include "tcp_demo.h"
#include "FileTransferReceiver.h"

extern ring_buffer rx_buf;
extern uint8_t g_WiFi_Shell_Enable;

/*
	argc : 输入的命令数量(以空格区分)
	argv : 命令的内容
*/
static int InitWifi_f(int argc, char **argv)
{
	CPU_TS_TmrInit();//初始化DWT计数器，用于延时函数
	ESP8266_Init();//相关GPIO/UART初始化
	//ESP8266_StaTcpClient_Unvarnish_ConfigTest();//通信配置
	ESP8266_ApTcpServer_ConfigTest();
	
	while(!g_WiFi_Shell_Enable)
	{
		ESP8266_CheckConnectTest();
	}
	unsigned char d;
	while(ring_buffer_read(&d, &rx_buf) == 0); 
	// 清空 ESP8266 接收缓冲区
	memset(strEsp8266_Fram_Record.Data_RX_BUF, 0, RX_BUF_MAX_LEN);
	strEsp8266_Fram_Record.InfBit.FramLength = 0;
	strEsp8266_Fram_Record.InfBit.FramFinishFlag = 0;
	
	Delay_ms(500);
	// --- 关键：倒计时前彻底清空缓冲区 ---
	while(ring_buffer_read(&d, &rx_buf) == 0);	

	return 0;
}
#if 0

static int InitWifi_f(int argc, char **argv)
{
	CPU_TS_TmrInit();//初始化DWT计数器，用于延时函数
	ESP8266_Init();//相关GPIO/UART初始化
	//ESP8266_StaTcpClient_Unvarnish_ConfigTest();//通信配置
	ESP8266_ApTcpServer_ConfigTest();
	printf("\r\nWiFi Shell Ready!\r\n");	
	unsigned char d;

	while(ring_buffer_read(&d, &rx_buf) == 0); 	

	return 0;
}
#endif

extern FileReceiver fileReceiver ;
static int InitEthernet_f(int argc, char **argv)
{
	reset_w5500();			/*硬复位W5500*/
	
	set_w5500_mac();		/*配置MAC地址*/
	set_w5500_ip();			/*配置IP地址*/
	
	socket_buf_init(txsize, rxsize);	/*初始化8个Socket的发送接收缓存大小*/
	
	fileReceiverInit(&fileReceiver);//初始化文件接收器

	while (getSn_SR(SOCK_TCPS) != SOCK_ESTABLISHED)
	{
		//等待PC客户端连接
		do_tcp_server(); //目前只是确认W5500的状态，其中的数据回环测试程序被我移出	
	}

	return 0;
}

/*
help 指令会输出的短说明
	"Init",
		
	"Init Esp-01s or W5500\r\n",
help 指令会输出的短说明
	"Usage: \r\n"
	"       ESP_01s_Init\r\n"
	"       W5500_Init\r\n",

通过函数指针调用的命令
*/



struct command InitWifi_cmd = {
	"InitWifi",
		
	"Init Esp-01s \r\n",
	
	"Usage: \r\n"
	"       ESP_01s_Init\r\n",

	InitWifi_f,
};


struct command InitEthernet_cmd = {
	"InitEthnet",
		
	"Init W5500 \r\n",
	
	"Usage: \r\n"
	"       W5500_Init\r\n",

	InitEthernet_f,
};

