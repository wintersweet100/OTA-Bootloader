#include "filetransferprotocol.h"
#include <QDebug>

//将数据封装成帧格式
QByteArray FileTransferProtocol::createFrame(uint8_t type, const QByteArray &data) {
    QByteArray frame;                // 创建空字节数组存储帧
    frame.append((char)0xAA);                   // 帧头标志（起始字节）
    frame.append((char)type);                   // 帧类型（FILE_START/FILE_DATA等）

    uint16_t len = data.size();                 // 获取数据长度
    frame.append((char)(len >> 8));             // 高字节（大端序）
    frame.append((char)(len & 0xFF));           // 低字节
    frame.append(data);                         // 追加实际数据

    uint16_t crc = calculateCRC16(frame);       // 计算CRC校验值
    frame.append((char)(crc >> 8));             // CRC高字节
    frame.append((char)(crc & 0xFF));           // CRC低字节
    frame.append((char)0xBB);                   // 帧尾标志（结束字节）

    return frame;
}

//解析接收到的帧
bool FileTransferProtocol::parseFrame(const QByteArray &rawData, Frame &frame) {
    if (rawData.size() < 7)                     // 最小帧长度检查（1+1+2+2+1=7）
        return false;

    int pos = 0;
    if ((uint8_t)rawData[pos++] != 0xAA)        // 检查帧头，pos移动到1
        return false;

    frame.type = (uint8_t)rawData[pos++];       // 读取类型，pos移动到2
    // 验证frame.type是否为有效的枚举值
    if (frame.type != FILE_START &&
        frame.type != FILE_DATA &&
        frame.type != FILE_END &&
        frame.type != ACK &&
        frame.type != NAK &&
        frame.type != READY &&
        frame.type != ERROR){
        return false;
    }

    frame.len = ((uint8_t)rawData[pos] << 8) | (uint8_t)rawData[pos + 1];  // 读取长度（大端序）
    pos += 2;                                   // pos移动到4
    if (rawData.size() < pos + frame.len + 2 + 1)  // 检查总长度是否足够
        return false;
    frame.data = rawData.mid(pos, frame.len);   // 提取数据段
    pos += frame.len;                           // pos移动到数据末尾
    frame.crc = ((uint8_t)rawData[pos] << 8) | (uint8_t)rawData[pos + 1];  // 读取CRC
    pos += 2;                                   // pos移动到CRC末尾
    if ((uint8_t)rawData[pos] != 0xBB)          // 检查帧尾
        return false;

    QByteArray crcData = rawData.mid(0, pos - 2);  // 提取需要校验的数据（不含CRC和EOF）
    uint16_t calculatedCrc = calculateCRC16(crcData);  // 重新计算CRC
    if (calculatedCrc != frame.crc){             // 对比CRC
        qDebug() << "CRC mismatch! Expected:" << frame.crc << "Got:" << calculatedCrc;
        return false;
     }
    return true;
}

//CRC校验
uint16_t FileTransferProtocol::calculateCRC16(const QByteArray &data) {
    uint16_t crc = 0xFFFF;                      // 初始值0xFFFF
    for (uint8_t byte : data) {                 // 遍历每个字节
        crc ^= byte;                            // 与当前字节异或
        for (int i = 0; i < 8; i++) {           // 处理8位
            if (crc & 1)                        // 最低位为1
                crc = (crc >> 1) ^ 0xA001;      // 右移并异或多项式
            else
                crc >>= 1;                      // 仅右移
        }
    }
    return crc;                // 返回最终CRC值
}

//生成文件头(文件名+文件大小)
QByteArray FileTransferProtocol::createFileHeader(const QString &fileName, uint32_t fileSize) {
    QByteArray header;
    QByteArray nameBytes = fileName.toUtf8();   // 文件名转UTF-8
    if (nameBytes.size() > 256)
        nameBytes = nameBytes.left(256);        // 限制最多256字节
    header.append((char)nameBytes.size());      // 文件名长度（1字节）
    header.append(nameBytes);                   // 文件名内容
    header.append((char)(fileSize >> 24));      // 文件大小高字节
    header.append((char)(fileSize >> 16));
    header.append((char)(fileSize >> 8));
    header.append((char)(fileSize & 0xFF));     // 文件大小低字节（共4字节，大端序）

    return header;
}

//辅助函数
QByteArray FileTransferProtocol::createAckFrame() {
    return createFrame(ACK, QByteArray());// 创建空数据的ACK帧
}

QByteArray FileTransferProtocol::createNakFrame() {
    return createFrame(NAK, QByteArray());
}

QByteArray FileTransferProtocol::createReadyFrame() {
    return createFrame(READY, QByteArray());
}
