#ifndef  __ESP8266_TEST_H
#define	 __ESP8266_TEST_H

#include "stm32f1xx.h"


/********************************** 用户需要设置的参数**********************************/
#if 1
#define      macUser_ESP8266_ApSsid                       "ilovelearning"                //要连接的热点的名称
#define      macUser_ESP8266_ApPwd                        "lzhnzswtanl#1007"           //要连接的热点的密钥

#define      macUser_ESP8266_TcpServer_IP                 "192.168.127.23"      //要连接的服务器的 IP
#define      macUser_ESP8266_TcpServer_Port               "8000"               //要连接的服务器的端口
#endif

#if 0
#define      macUser_ESP8266_ApSsid                       "i-Star"                //要连接的热点的名称
#define      macUser_ESP8266_ApPwd                        "lzhnzswtanl#1007"           //要连接的热点的密钥
#define      macUser_ESP8266_TcpServer_IP                 "10.19.164.29"      //要连接的服务器的 IP
#define      macUser_ESP8266_TcpServer_Port               "8000"               //要连接的服务器的端口
#endif


#define  	 macUser_ESP8266_BulitApSsid     			  "BinghuoLink"      //要建立的热点的名称
#define  	 macUser_ESP8266_BulitApEcn      			   OPEN               //要建立的热点的加密方式
#define  	 macUser_ESP8266_BulitApPwd      			   "wildfire"         //要建立的热点的密钥

#define      macUser_ESP8266_TcpServer_OverTime   		  "1800"             //服务器超时时间（单位：秒）

/********************************** 外部全局变量 ***************************************/
extern volatile uint8_t ucTcpClosedFlag;


/********************************** 测试函数声明 ***************************************/
void ESP8266_StaTcpClient_Unvarnish_ConfigTest(void);
void ESP8266_ApTcpServer_ConfigTest(void);
void ESP8266_CheckConnectTest(void);
void ESP8266_CheckRecvDataTest(void);

#endif

