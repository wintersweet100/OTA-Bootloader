#ifndef FILETRANSFERPROTOCOL_H
#define FILETRANSFERPROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    FTP_START = 0x01,
    FTP_DATA = 0x02,
    FTP_END = 0x03,
    FTP_ACK = 0x04,
    FTP_NAK = 0x05,
    FTP_READY = 0x06,
    FTP_ERROR = 0x07
} FrameType;

typedef struct {
    uint8_t sof;
    uint8_t type;
    uint16_t len;
    uint8_t *data;
    uint16_t crc;
    uint8_t eof;
} Frame;

uint16_t calculateCRC16(const uint8_t *data, uint16_t len);
bool parseFrame(const uint8_t *rawData, uint16_t rawLen, Frame *frame);
void createFrame(uint8_t type, const uint8_t *data, uint16_t len, uint8_t *output, uint16_t *outputLen);

#endif	
