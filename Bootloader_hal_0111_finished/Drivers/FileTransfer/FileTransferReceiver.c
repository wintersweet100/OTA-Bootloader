#include "FileTransferReceiver.h"
#include <string.h>
#include <stdio.h>
#include "socket.h"
#include "w5500.h"
#include "bsp_esp8266.h"

FileReceiver fileReceiver;

extern  unsigned int g_download_address;//接收.bin文件的内存起始地址
extern  unsigned int g_bytes;//接收的.bin文件的字节数
extern uint8_t g_WiFi_Shell_Enable;//判断Wifi连接状态
extern UART_HandleTypeDef huart_esp8266;

// 内存写入函数
void writeToMemory(uint32_t addr, const uint8_t *data, uint32_t len);
// 获取接收大小
uint32_t getReceivedSize(void);
// 获取下载地址
uint32_t getDownloadAddress(void);


void fileReceiverInit(FileReceiver *receiver) {
    receiver->state = RECV_IDLE;
    receiver->rxPos = 0;
    receiver->fileSize = 0;
    receiver->receivedBytes = 0;
    memset(receiver->fileName, 0, sizeof(receiver->fileName));
}
void fileReceiverHandleData(FileReceiver *receiver, const uint8_t *data, uint16_t len) {
    // 逐字节追加到缓冲区，边接收边解析
    for (uint16_t i = 0; i < len; i++) {
        receiver->rxBuffer[receiver->rxPos++] = data[i];
        
        // 缓冲区满了但还没解析出完整帧，说明帧太大或数据错误
        if (receiver->rxPos >= RX_BUFFER_SIZE) {
            receiver->state = RECV_ERROR;
            receiver->rxPos = 0;
            return;
        }
        
        // ★ 关键改进：用 while 替代 i = 0，避免数据重复
        while (receiver->rxPos >= 7) {
            // 检查帧头
            if (receiver->rxBuffer[0] != 0xAA) {
                // 寻找下一个 0xAA
                uint16_t j = 1;
                while (j < receiver->rxPos && receiver->rxBuffer[j] != 0xAA) {
                    j++;
                }
                if (j < receiver->rxPos) {
                    receiver->rxPos -= j;
                    memmove(receiver->rxBuffer, &receiver->rxBuffer[j], receiver->rxPos);
                    continue;  // 继续检查新的帧头
                } else {
                    receiver->rxPos = 0;
                    break;  // 等待更多数据
                }
            }
            
            // 尝试解析帧
            Frame frame;
            if (parseFrame(receiver->rxBuffer, receiver->rxPos, &frame)) {
                uint16_t frameLen = 1 + 1 + 2 + frame.len + 2 + 1;
                
                // 处理帧
                processFrame(receiver, &frame);
                
                // 移除已处理的帧
                receiver->rxPos -= frameLen;
                if (receiver->rxPos > 0) {
                    memmove(receiver->rxBuffer, &receiver->rxBuffer[frameLen], receiver->rxPos);
                }
                // 继续处理下一个帧（while 会自动重新检查）
            } else {
                // 帧不完整，等待更多数据
                break;
            }
        }
    }
}

#if 0
void fileReceiverHandleData(FileReceiver *receiver, const uint8_t *data, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        if (receiver->rxPos >= RX_BUFFER_SIZE) {
            receiver->state = RECV_ERROR;
            receiver->rxPos = 0;
            return;
        }
        receiver->rxBuffer[receiver->rxPos++] = data[i];
        
        // 尝试解析帧（只要缓冲区中有足够数据）
        while (receiver->rxPos >= 7) {
            // 检查帧头
            if (receiver->rxBuffer[0] != 0xAA) {
                // 寻找下一个 0xAA
                uint16_t j = 1;
                while (j < receiver->rxPos && receiver->rxBuffer[j] != 0xAA) {
                    j++;
                }
                if (j < receiver->rxPos) {
                    receiver->rxPos -= j;
                    memmove(receiver->rxBuffer, &receiver->rxBuffer[j], receiver->rxPos);
                    continue;  // 继续检查新的帧头
                } else {
                    receiver->rxPos = 0;
                    break;  // 等待更多数据
                }
            }
            
            // 尝试解析帧
            Frame frame;
            if (parseFrame(receiver->rxBuffer, receiver->rxPos, &frame)) {
                uint16_t frameLen = 1 + 1 + 2 + frame.len + 2 + 1;
                
                // 处理帧
                processFrame(receiver, &frame);
                
                // 移除已处理的帧
                receiver->rxPos -= frameLen;
                if (receiver->rxPos > 0) {
                    memmove(receiver->rxBuffer, &receiver->rxBuffer[frameLen], receiver->rxPos);
                }
                // 继续处理下一个帧
            } else {
                // 帧不完整，等待更多数据
                break;
            }
        }
    }
}
#endif

#if 0
//出错:QT端接收到了AT指令信息，而不是READY信号.
void fileReceiverHandleData(FileReceiver *receiver, const uint8_t *data, uint16_t len) {
    // 逐字节追加到缓冲区，边接收边解析
    for (uint16_t i = 0; i < len; i++) {
        if (receiver->rxPos >= RX_BUFFER_SIZE) {
            receiver->state = RECV_ERROR;
            receiver->rxPos = 0;
            return;
        }
        receiver->rxBuffer[receiver->rxPos++] = data[i];
        
        // ★ 关键：每加入一个字节就尝试解析（参考W5500逻辑）
        if (receiver->rxPos >= 7) {  // 最小帧长度
            // 检查帧头
            if (receiver->rxBuffer[0] != 0xAA) {
                // 寻找下一个 0xAA
                uint16_t j = 1;
                while (j < receiver->rxPos && receiver->rxBuffer[j] != 0xAA) {
                    j++;
                }
                if (j < receiver->rxPos) {
                    receiver->rxPos -= j;
                    memmove(receiver->rxBuffer, &receiver->rxBuffer[j], receiver->rxPos);
                    i = 0;  // ★ 重新检查缓冲区（参考W5500的 i = 0）
                    continue;
                } else {
                    receiver->rxPos = 0;
                    return;
                }
            }
            
            // 尝试解析帧
            Frame frame;
            if (parseFrame(receiver->rxBuffer, receiver->rxPos, &frame)) {
                uint16_t frameLen = 1 + 1 + 2 + frame.len + 2 + 1;
                
                // 处理帧
                processFrame(receiver, &frame);
                
                // 移除已处理的帧
                receiver->rxPos -= frameLen;
                if (receiver->rxPos > 0) {
                    memmove(receiver->rxBuffer, &receiver->rxBuffer[frameLen], receiver->rxPos);
                }
                // ★ 继续处理下一个帧（可能在缓冲区中）
                i = 0;  // 让循环重新检查缓冲区
            }
        }
    }
}
#endif

#if 0
//仅可传输小字节的.bin文件，无法传输超过缓冲区大小的，较大字节的.bin文件
void fileReceiverHandleData(FileReceiver *receiver, const uint8_t *data, uint16_t len) {
    // 逐字节追加到缓冲区
    for (uint16_t i = 0; i < len; i++) {
        if (receiver->rxPos >= RX_BUFFER_SIZE) {
            receiver->state = RECV_ERROR;
            receiver->rxPos = 0;
            return;
        }
        receiver->rxBuffer[receiver->rxPos++] = data[i];
   }
    
    // ★ 关键改进：循环处理所有可解析的帧
    while (receiver->rxPos >= 7) {
        // 检查帧头
        if (receiver->rxBuffer[0] != 0xAA) {
            // 寻找下一个 0xAA
            uint16_t j = 1;
            while (j < receiver->rxPos && receiver->rxBuffer[j] != 0xAA) {
                j++;
            }
            if (j < receiver->rxPos) {
                receiver->rxPos -= j;
                memmove(receiver->rxBuffer, &receiver->rxBuffer[j], receiver->rxPos);
                continue;  // ★ 改为 continue，继续检查新的帧头
            } else {
                receiver->rxPos = 0;
                return;
            }
        }
        
        // 尝试解析帧
        Frame frame;
        if (parseFrame(receiver->rxBuffer, receiver->rxPos, &frame)) {
            uint16_t frameLen = 1 + 1 + 2 + frame.len + 2 + 1;
            
            // 处理帧
            processFrame(receiver, &frame);
            
            // 移除已处理的帧
            receiver->rxPos -= frameLen;
            if (receiver->rxPos > 0) {
                memmove(receiver->rxBuffer, &receiver->rxBuffer[frameLen], receiver->rxPos);
            }
            // 继续处理下一个帧
        } else {
            // 帧不完整，等待更多数据
            break;
			
		}
	}
}
#endif

#if 0
void fileReceiverHandleData(FileReceiver *receiver, const uint8_t *data, uint16_t len) {

//    printf("[Receiver] Received %d bytes\r\n", len);
//    printf("[Receiver] Hex: ");
//    for (uint16_t i = 0; i < len && i < 50; i++) {
//        printf("%02X ", data[i]);
//    }
//    if (len > 50) printf("... (total %d bytes)", len);
//    printf("\r\n");


    // 逐字节追加到缓冲区，边接收边解析
    for (uint16_t i = 0; i < len; i++) {
   
        // 缓冲区满了但还没解析出完整帧，说明帧太大或数据错误
        if (receiver->rxPos >= RX_BUFFER_SIZE) {
            //printf("[Receiver] RX buffer full without complete frame!\n");
            receiver->state = RECV_ERROR;
            receiver->rxPos = 0;
            return;
        }

        receiver->rxBuffer[receiver->rxPos++] = data[i];
     } 

    // ★ 关键：循环尝试解析缓冲区中的帧
    while (receiver->rxPos >= 7) {
        // 检查帧头
        if (receiver->rxBuffer[0] != 0xAA) {
//            printf("[fileReceiverHandleData] Invalid frame header at pos 0: 0x%02X\r\n", 
//                   receiver->rxBuffer[0]);
//            printf("[fileReceiverHandleData] Buffer content: ");
//            for (uint16_t i = 0; i < receiver->rxPos && i < 20; i++) {
//                printf("%02X ", receiver->rxBuffer[i]);
//            }
//            printf("\r\n");
            
            // ★ 寻找下一个 0xAA
            uint16_t j = 1;
            while (j < receiver->rxPos && receiver->rxBuffer[j] != 0xAA) {
                j++;
            }
            
            if (j < receiver->rxPos) {
//                printf("[fileReceiverHandleData] Found next 0xAA at pos %d, removing %d bytes\r\n", j, j);
                receiver->rxPos -= j;
                memmove(receiver->rxBuffer, &receiver->rxBuffer[j], receiver->rxPos);
            } else {
//                printf("[fileReceiverHandleData] No 0xAA found, clearing buffer\r\n");
                receiver->rxPos = 0;
                return;
            }
        }

		// 尝试解析帧（只要缓冲区中有足够数据）
        Frame frame;
        
        if (parseFrame(receiver->rxBuffer, receiver->rxPos, &frame)) {
//            printf("[fileReceiverHandleData] Frame parsed: type=0x%02X, len=%d\r\n", 
//                   frame.type, frame.len);
            
            // 计算帧长度
            uint16_t frameLen = 1 + 1 + 2 + frame.len + 2 + 1;  // AA + Type + Len + Data + CRC + BB
            
            // 处理帧
            processFrame(receiver, &frame);
            
            // 移除已处理的帧
            receiver->rxPos -= frameLen;
            if (receiver->rxPos > 0) {
                memmove(receiver->rxBuffer, &receiver->rxBuffer[frameLen], receiver->rxPos);
//                printf("[fileReceiverHandleData] Removed %d bytes, %d bytes remaining\r\n", 
//                       frameLen, receiver->rxPos);
            } else {
//                printf("[fileReceiverHandleData] Buffer cleared\r\n");
            }
            
            // 继续处理下一个帧
        } else {
            // parseFrame 失败，可能是帧不完整
//            printf("[fileReceiverHandleData] parseFrame failed, buffer has %d bytes\r\n", 
//                   receiver->rxPos);
            
            // 检查是否是帧不完整
//            if (receiver->rxPos >= 4) {
//                uint16_t expectedLen = ((uint16_t)receiver->rxBuffer[2] << 8) | receiver->rxBuffer[3];
//                uint16_t totalNeeded = 1 + 1 + 2 + expectedLen + 2 + 1;
//                printf("[fileReceiverHandleData] Expected total frame length: %d, have: %d\r\n", 
//                       totalNeeded, receiver->rxPos);
//            }
            
            break;  // 等待更多数据
        }
	}
}
#endif

#if 1
void fileReceiverHandleData_W5500(FileReceiver *receiver, const uint8_t *data, uint16_t len) {


    // 逐字节追加到缓冲区，边接收边解析
    for (uint16_t i = 0; i < len; i++) {
        receiver->rxBuffer[receiver->rxPos++] = data[i];
        
        // 缓冲区满了但还没解析出完整帧，说明帧太大或数据错误
        if (receiver->rxPos >= RX_BUFFER_SIZE) {
            //printf("[Receiver] RX buffer full without complete frame!\n");
            receiver->state = RECV_ERROR;
            receiver->rxPos = 0;
            return;
        }
        
        // 尝试解析帧（只要缓冲区中有足够数据）
        if (receiver->rxPos >= 7) {  // 最小帧长度：SOF(1) + Type(1) + Len(2) + CRC(2) + EOF(1)
            Frame frame;
            uint16_t frameLen = 0;
            
            if (parseFrame(receiver->rxBuffer, receiver->rxPos, &frame)) {

                // 计算实际帧长度
                frameLen = 1 + 1 + 2 + frame.len + 2 + 1;
                
                // 处理帧
                processFrame(receiver, &frame);
                
                // 移除已处理的帧
                receiver->rxPos -= frameLen;
                if (receiver->rxPos > 0) {
                    memmove(receiver->rxBuffer, &receiver->rxBuffer[frameLen], receiver->rxPos);
                }
                // 继续处理下一个帧（可能在缓冲区中）
                i = 0;  // 让循环重新检查缓冲区
            }
        }
    }
}
#endif


// 单独提取帧处理逻辑
void processFrame(FileReceiver *receiver, Frame *frame) {
    switch (frame->type) {
        case FTP_READY:
            fileReceiverSendReady(receiver);
            receiver->state = RECV_WAITING_FILE_START;
            break;
            
        case FTP_START: {
            uint8_t nameLen = frame->data[0];
            memcpy(receiver->fileName, &frame->data[1], nameLen);
            receiver->fileName[nameLen] = '\0';
            receiver->fileSize = ((uint32_t)frame->data[1 + nameLen] << 24) |
                                 ((uint32_t)frame->data[2 + nameLen] << 16) |
                                 ((uint32_t)frame->data[3 + nameLen] << 8) |
                                 frame->data[4 + nameLen];
            receiver->receivedBytes = 0;
            receiver->state = RECV_RECEIVING_DATA;
//            printf("[Receiver] FILE_START: %s, size=%lu\n", receiver->fileName, receiver->fileSize);
            fileReceiverSendAck(receiver);
            break;
        }
        
        case FTP_DATA: {
            if (frame->len > 4096) {
                receiver->state = RECV_ERROR;
                fileReceiverSendNak(receiver);
//                printf("[Receiver] Block too large: %d bytes\n", frame->len);
                break;
            }
            
            if (receiver->receivedBytes + frame->len > 0x200bffff) {
                receiver->state = RECV_ERROR;
                fileReceiverSendNak(receiver);
                printf("[Receiver] Total size exceeds limit!\n");
                break;
            }
            
            uint32_t writeAddr = g_download_address + receiver->receivedBytes;
            writeToMemory(writeAddr, frame->data, frame->len);
            receiver->receivedBytes += frame->len;
            g_bytes = receiver->receivedBytes;
            
//            if (receiver->receivedBytes % 102400 == 0 || receiver->receivedBytes == receiver->fileSize) {
//                printf("[Receiver] Progress: %lu/%lu bytes\n", receiver->receivedBytes, receiver->fileSize);
//            }
            
            fileReceiverSendAck(receiver);
            break;
        }
        
        case FTP_END:
            receiver->state = RECV_COMPLETE;
            g_bytes = receiver->receivedBytes;
//            printf("[Receiver] FILE_END: transfer complete, total %lu bytes\n", receiver->receivedBytes);
            fileReceiverSendAck(receiver);
            break;
            
        default:
            printf("[Receiver] Unknown frame type: %d\n", frame->type);
    }
}

#if 0
void fileReceiverHandleData(FileReceiver *receiver, const uint8_t *data, uint16_t len) {
    // 将数据追加到接收缓冲区
    if (receiver->rxPos + len > RX_BUFFER_SIZE) {
        receiver->state = RECV_ERROR;
		printf("[Receiver] RX buffer overflow!\n");
        return;
    }
    
    memcpy(&receiver->rxBuffer[receiver->rxPos], data, len);
    receiver->rxPos += len;

	//解析QT端发来的帧数据
	while(receiver->rxPos > 0 ){
	    Frame frame;	
		if (!parseFrame(receiver->rxBuffer, receiver->rxPos, &frame)) {
			// 帧不完整，继续等待
			break;
		}
		
		// 计算已解析的帧长度
		// SOF + Type + Len + Data + CRC + EOF
        uint16_t frameLen = 1 + 1 + 2 + frame.len + 2 + 1; 
		
		switch (frame.type) {
			case FTP_READY:{
//				printf("[Receiver] Received READY from PC,sendinf READY back\n");
				fileReceiverSendReady(receiver);
				receiver->state = RECV_WAITING_FILE_START;
				break;
			}

			case FTP_START: {
				// 解析文件头：[NameLen][FileName...][Size_H][Size_MH][Size_ML][Size_L]
				uint8_t nameLen = frame.data[0];
				memcpy(receiver->fileName, &frame.data[1], nameLen);
				receiver->fileName[nameLen] = '\0';
				
				receiver->fileSize = ((uint32_t)frame.data[1 + nameLen] << 24) |
									 ((uint32_t)frame.data[2 + nameLen] << 16) |
									 ((uint32_t)frame.data[3 + nameLen] << 8) |
									 frame.data[4 + nameLen];
				
				receiver->receivedBytes = 0;
				receiver->state = RECV_RECEIVING_DATA;
				
//				printf("[Receiver] FILE_START: %s, size=%lu\n", receiver->fileName, receiver->fileSize);
				
				// 发送ACK
				fileReceiverSendAck(receiver);
				break;
			}
			
			case FTP_DATA: {
				
				// 检查不是缓冲区大小，而是单个块大小
                if (frame.len > 4096) {  // 单块数据不超过4KB
                    receiver->state = RECV_ERROR;
                    fileReceiverSendNak(receiver);
                    printf("[Receiver] Block too large: %d bytes\n", frame.len);
                    break;
                }
                
                // 检查总大小不超过可用内存
                if (receiver->receivedBytes + frame.len > 0x200bffff) {  // 1MB限制
                    receiver->state = RECV_ERROR;
                    fileReceiverSendNak(receiver);
                    printf("[Receiver] Total size exceeds limit!\n");
                    break;
                }

				
                uint32_t writeAddr = g_download_address + receiver->receivedBytes;
                writeToMemory(writeAddr, frame.data, frame.len);				

				receiver->receivedBytes += frame.len;
				g_bytes = receiver->receivedBytes;  // 同时更新 g_bytes			
	
//				printf("[Receiver] FILE_DATA: received %lu/%lu bytes\n", 
//						   receiver->receivedBytes, receiver->fileSize);
				// 每接收100KB打印一次进度
//                if (receiver->receivedBytes % 102400 == 0 || 
//                    receiver->receivedBytes == receiver->fileSize) {
//                    printf("[Receiver] Progress: %lu/%lu bytes\n", 
//                           receiver->receivedBytes, receiver->fileSize);
//                }	
				

				// 发送ACK
				fileReceiverSendAck(receiver);
				break;
			}
			
			case FTP_END: {
				receiver->state = RECV_COMPLETE;
				g_bytes = receiver->receivedBytes;  //  最终更新 g_bytes
//				printf("[Receiver] FILE_END: transfer complete, total %lu bytes\n", 
//						   receiver->receivedBytes);
				
				// 发送最后的ACK
				fileReceiverSendAck(receiver);
				
				// 这里可以将 fileBuffer 中的数据写入Flash
				// writeToFlash(receiver->fileBuffer, receiver->receivedBytes);
				break;
			}
			
			default:
				printf("[Receiver] Unknown frame type: %d\n", frame.type);
		}
		
		        // ★ 移除已处理的帧，继续处理下一个
        receiver->rxPos -= frameLen;
        if (receiver->rxPos > 0) {
            memmove(receiver->rxBuffer, &receiver->rxBuffer[frameLen], receiver->rxPos);
        }
		
	}

}
#endif

void fileReceiverSendReady(FileReceiver *receiver) {
    uint8_t txBuffer[10];
    uint16_t txLen;
    createFrame(FTP_READY, NULL, 0, txBuffer, &txLen);

//    printf("[Sender] Preparing READY frame, len=%d: ", txLen);
//    for (uint16_t i = 0; i < txLen; i++) {
//        printf("%02X ", txBuffer[i]);
//    }
//    printf("\n");

	if(getSn_SR(SOCK_TCPS) == SOCK_ESTABLISHED){
		uint16_t sent = send(SOCK_TCPS,txBuffer,txLen);
	}
	else if(g_WiFi_Shell_Enable){
		//HAL_UART_Transmit(&huart_esp8266, txBuffer, txLen, 100);
		//ESP8266_SendString2(DISABLE,txBuffer,txLen,Multiple_ID_0);
		//ESP8266_SendString(DISABLE,(char * )txBuffer,txLen,Multiple_ID_0);
		ESP8266_SendBinary(txBuffer, txLen, Multiple_ID_0);
	}
	else{

	}
	/*发送前检查连接状态getSn SR(SOCK TCPS)== SOCK ESTABLISHED
	不需要缓冲区，直接发送(帧数据很小，7-10字节)*/
    //printf("[Receiver] Sent READY\n");
}

void fileReceiverSendAck(FileReceiver *receiver) {
    uint8_t txBuffer[10];
    uint16_t txLen;
    createFrame(FTP_ACK, NULL, 0, txBuffer, &txLen);
    if(getSn_SR(SOCK_TCPS) == SOCK_ESTABLISHED){
		send(SOCK_TCPS,txBuffer,txLen);
	}
	else if(g_WiFi_Shell_Enable){
		ESP8266_SendBinary(txBuffer, txLen, Multiple_ID_0);
	}
}

void fileReceiverSendNak(FileReceiver *receiver) {
    uint8_t txBuffer[10];
    uint16_t txLen;
    createFrame(FTP_NAK, NULL, 0, txBuffer, &txLen);
    if(getSn_SR(SOCK_TCPS) == SOCK_ESTABLISHED){
		send(SOCK_TCPS,txBuffer,txLen);
	}
	else if(g_WiFi_Shell_Enable){
		ESP8266_SendBinary(txBuffer, txLen, Multiple_ID_0);
	}
}


// 内存写入函数
void writeToMemory(uint32_t addr, const uint8_t *data, uint32_t len) {
    uint8_t *pdata = (uint8_t *)addr;
    for (uint32_t i = 0; i < len; i++) {
        pdata[i] = data[i];
    }
}

// 获取接收大小
uint32_t getReceivedSize(void) {
    return fileReceiver.receivedBytes;
}

// 获取下载地址
uint32_t getDownloadAddress(void) {
    return g_download_address;
}

