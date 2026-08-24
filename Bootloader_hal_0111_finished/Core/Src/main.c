/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "shell.h"
#include "flash_cmd.h"
#include "ring_buffer.h"
#include "bsp_esp8266.h"
#include "bsp_esp8266_test.h"
#include "core_delay.h"
#include "w5500.h"
#include "W5500_conf.h"
#include "socket.h"
#include "utility.h"
#include "tcp_demo.h"
#include "string.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern void start_app(unsigned int vector); 
typedef unsigned int __be32;
typedef unsigned char uint8_t;
extern struct flash_ops *get_flash(void);
extern ring_buffer rx_buf;
extern uint8_t g_WiFi_Shell_Enable;
#define IH_NMLEN 32

typedef struct image_header {
    __be32 ih_magic;           /* Image Header Magic Number	*/
    __be32 ih_hcrc;            /* Image Header CRC Checksum	*/
    __be32 ih_time;            /* Image Creation Timestamp	*/
    __be32 ih_size;            /* Image Data Size		??*/
    __be32 ih_load;            /* Data	 Load  Address	??*/
    __be32 ih_ep;              /* Entry Point Address		*/
    __be32 ih_dcrc;            /* Image Data CRC Checksum	*/
    uint8_t ih_os;             /* Operating System		*/
    uint8_t ih_arch;           /* CPU architecture		*/
    uint8_t ih_type;           /* Image Type			*/
    uint8_t ih_comp;           /* Compression Type		*/
    uint8_t ih_name[IH_NMLEN]; /* Image Name		*/
} image_header_t;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/*将大端字节序转换为小端(大端：人读;小端：机读) */
unsigned int be32_to_cpu(unsigned int big32) {

    unsigned char *p = (unsigned char *)&big32;
    return (unsigned int)(p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3]);
}
void copy_app(int *from, int *to, int len) {
    // 从哪里到哪里, 多长 ?
    int i;
    for (i = 0; i < len / 4 + 1; i++) {
        to[i] = from[i];
    }
}
void relocate_and_startappp(unsigned int pos) {

    image_header_t head;
    unsigned int load;
    unsigned int size;
    unsigned int new_pos = pos + sizeof(image_header_t);
	struct flash_ops *fp = get_flash();
	unsigned int ret;

    /*读出头部 */
    //header = (image_header_t *)pos;
	ret = fp->read((unsigned char *)&head, pos, sizeof(image_header_t));
	if (ret != sizeof(image_header_t))
	{
		putstr("can not read header\r\n");
		return;
	}

    /*解析头部*/
    load = be32_to_cpu(head.ih_load);
    size = be32_to_cpu(head.ih_size);

    printf("load addr: ");
    printf("0x%08X", load);
    printf("\r\n");
    printf("size addr: ");
    printf("0x%08X", size);
    printf("\r\n");

    /*把程序复制到RAM*/
    //copy_app((int *)new_pos, (int *)load, size);
	ret = fp->read((unsigned char *)load, new_pos, size);

	printf("copy success");
    /*跳转执行 */
    start_app(load);
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  unsigned int postion = 0x08009000;
  int delay_cnt = 5;
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM6_Init();
  MX_USART1_UART_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim6); // IT 表示 Interrupt，这会同时开启计数器和中断
  /*-----------------ESP-01S配置--start---esp作为客户端连接我的wifi----------------------------------*/

  /*-----------------ESP-01S配置-end----------------------------------------------------------------*/


  /*-----------------ESP-01S配置--start---esp作为服务器----------------------------------*/
#if 0
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
#endif	
  /*-----------------ESP-01S配置-end----------------------------------------------------------------*/


  /*-----------------W5500配置-start----------------------------------------------------------------*/
	systick_init(72);		/*初始化Systick工作时钟*/
#if 0
	reset_w5500();			/*硬复位W5500*/

	set_w5500_mac();		/*配置MAC地址*/
	set_w5500_ip();			/*配置IP地址*/

	socket_buf_init(txsize, rxsize);	/*初始化8个Socket的发送接收缓存大小*/
	while (getSn_SR(SOCK_TCPS) != SOCK_ESTABLISHED)
	{
		//等待PC客户端连接
		do_tcp_server(); //目前只是确认W5500的状态，其中的数据回环测试程序被我移出	
	}
#endif
  /*-----------------W5500配置-end----------------------------------------------------------------*/

  printf("\r\nbootloader \r\n");
  printf("Press space to start shell:%d\r\n",delay_cnt);

  while (delay_cnt > 0)
  {
      // 1. 检查环形缓冲区是否有数据，而不是检查硬件标志位
      unsigned char temp_char;
      if (ring_buffer_read(&temp_char, &rx_buf) == 0) // 假设返回 0 表示成功读到数据
      {
          if (temp_char == ' ')
          {
              printf("\r\nEntering shell...\r\n");
              shell();
              break; 
          }
		  else
		  {
			  printf("[0x%02X]", temp_char);	
		  }

      }
      // 2. 这里的延时需要拆分，或者配合计数器
      // 如果直接 Delay 1000ms，按键响应会很迟钝
      // 建议每 100ms 检查一次按键，循环 10 次
      for(int i = 0; i < 100; i++) 
      {
          //HAL_Delay(100); // 或者使用你的 Delay_ms_TIM6(100)
		  Delay_ms_TIM6(10);
          if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) == SET) break; 
      }

      delay_cnt--;
      printf("%d\r\n", delay_cnt); // 更新显示
  }

  printf("\r\nStart APP ...\r\n");
  /*-----------------直接跳转到工作区-start----------------------------------------------------------------*/
  // 禁用所有中断（防止 Bootloader 中断干扰 App）
  __disable_irq();
  HAL_TIM_Base_Stop_IT(&htim6); // 先关掉你开启的定时器
  // 关闭 SysTick、清除挂起的中断（干净的执行环境）
  SysTick->CTRL = 0;
  SysTick->LOAD = 0;
  SysTick->VAL = 0;
  for (uint32_t i = 0; i < 8; i++)
  {
    NVIC->ICER[i] = 0xFFFFFFFF; // 禁用所有中断
    NVIC->ICPR[i] = 0xFFFFFFFF; // 清除挂起标志
  }

  //relocate_and_startappp(postion);
  printf("Jumping to address: 0x%08X\r\n", postion);
  start_app(postion);
 /*-----------------直接跳转到工作区-end----------------------------------------------------------------*/
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	printf("跳转失败\r\n");
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
