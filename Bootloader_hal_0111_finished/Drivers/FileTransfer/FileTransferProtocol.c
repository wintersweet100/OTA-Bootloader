#include "FileTransferProtocol.h"
#include <string.h>
#include "usart.h"

uint16_t calculateCRC16(const uint8_t *data, uint16_t len) {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

bool parseFrame(const uint8_t *rawData, uint16_t rawLen, Frame *frame) {
//	printf("[parseFrame] rawLen=%d\r\n", rawLen);

    if (rawLen < 7) {	
//		printf("[parseFrame] Too short\r\n");
		return false;
    }

    uint16_t pos = 0;
    
    if (rawData[pos++] != 0xAA) {
//        printf("[parseFrame] Invalid header: 0x%02X\r\n", rawData[0]);
		return false;
	}
    
    frame->type = rawData[pos++];
    frame->len = ((uint16_t)rawData[pos] << 8) | rawData[pos + 1];
    pos += 2;

//    printf("[parseFrame] type=0x%02X, len=%d, pos=%d\r\n", frame->type, frame->len, pos);
    
    if (rawLen < pos + frame->len + 3) {
//        printf("[parseFrame] Incomplete: need %d, have %d\r\n", pos + frame->len + 3, rawLen);		
		return false;
	}
    
    frame->data = (uint8_t *)&rawData[pos];
    pos += frame->len;
    
    frame->crc = ((uint16_t)rawData[pos] << 8) | rawData[pos + 1];
//    printf("[parseFrame] Received CRC: 0x%04X\r\n", frame->crc);

    pos += 2;
    
    if (rawData[pos] != 0xBB) {
//		printf("[parseFrame] Invalid footer: 0x%02X\r\n", rawData[pos]);
		return false;
	}
    
    uint16_t calcCrc = calculateCRC16(rawData, pos - 2);
//    printf("[parseFrame] Calculated CRC: 0x%04X\r\n", calcCrc);

    if (calcCrc != frame->crc){
//        printf("[parseFrame] CRC mismatch! Received: 0x%04X, Calculated: 0x%04X\r\n", 
//               frame->crc, calcCrc);
		return false;
	} 
    
//    printf("[parseFrame] Success!\r\n");
    return true;
}

void createFrame(uint8_t type, const uint8_t *data, uint16_t len, uint8_t *output, uint16_t *outputLen) {
    uint16_t pos = 0;
    output[pos++] = 0xAA;
    output[pos++] = type;
    output[pos++] = (len >> 8) & 0xFF;
    output[pos++] = len & 0xFF;
    
    if (data && len > 0) {
        memcpy(&output[pos], data, len);
        pos += len;
    }
    
    uint16_t crc = calculateCRC16(output, pos);
    output[pos++] = (crc >> 8) & 0xFF;
    output[pos++] = crc & 0xFF;
    output[pos++] = 0xBB;
    
    *outputLen = pos;

	//µ÷ÊÔÐÅÏ¢
//	printf("[createFrame] Type=0x%02X,DataLen=%d,TotalLen=%d\n",type,len,pos);
}


