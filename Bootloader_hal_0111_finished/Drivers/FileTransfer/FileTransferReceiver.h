#ifndef FILETRANSFERRECEIVER_H
#define FILETRANSFERRECEIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "FileTransferProtocol.h"

//#define RX_BUFFER_SIZE    2048
//#define FILE_BUFFER_SIZE  32768  // 32KB
//#define DownLoad_Address 0x20000000
#define RX_BUFFER_SIZE    2048
#define FILE_BUFFER_SIZE  2048  // 32KB


typedef enum {
    RECV_IDLE,
    RECV_WAITING_FILE_START,
    RECV_RECEIVING_DATA,
    RECV_COMPLETE,
    RECV_ERROR
} RecvState;

typedef struct {
    RecvState state;
    uint8_t rxBuffer[RX_BUFFER_SIZE];
    uint16_t rxPos;
    uint8_t fileBuffer[FILE_BUFFER_SIZE];
    uint32_t fileSize;
    uint32_t receivedBytes;
    char fileName[256];
} FileReceiver;


void fileReceiverInit(FileReceiver *receiver);
void fileReceiverHandleData(FileReceiver *receiver, const uint8_t *data, uint16_t len);
void fileReceiverHandleData_W5500(FileReceiver *receiver, const uint8_t *data, uint16_t len);
// 单独提取帧处理逻辑
void processFrame(FileReceiver *receiver, Frame *frame);
void fileReceiverSendReady(FileReceiver *receiver);
void fileReceiverSendAck(FileReceiver *receiver);
void fileReceiverSendNak(FileReceiver *receiver);

#endif
