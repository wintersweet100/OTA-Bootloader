#ifndef FILETRANSFERPROTOCOL_H
#define FILETRANSFERPROTOCOL_H

#include <QWidget>
#include <QByteArray>
#include <QString>
#include <cstdint>

class FileTransferProtocol : public QWidget
{
    Q_OBJECT
public:
    explicit FileTransferProtocol(QWidget *parent = nullptr);

    // 帧类型
    enum FrameType : uint8_t {
        FILE_START = 0x01,
        FILE_DATA = 0x02,
        FILE_END = 0x03,
        ACK = 0x04,
        NAK = 0x05,
        READY = 0x06,
        ERROR = 0x07
    };

    // 帧结构
    struct Frame {
        uint8_t sof;        // 0xAA
        uint8_t type;
        uint16_t len;
        QByteArray data;
        uint16_t crc;
        uint8_t eof;        // 0xBB
    };

    // 创建帧
    static QByteArray createFrame(uint8_t type, const QByteArray &data);

    // 解析帧
    static bool parseFrame(const QByteArray &rawData, Frame &frame);

    // CRC16 计算
    static uint16_t calculateCRC16(const QByteArray &data);

    // 创建文件头数据
    static QByteArray createFileHeader(const QString &fileName, uint32_t fileSize);

    // 创建 ACK 帧
    static QByteArray createAckFrame();

    // 创建 NAK 帧
    static QByteArray createNakFrame();

    // 创建 READY 帧
    static QByteArray createReadyFrame();

};

#endif // FILETRANSFERPROTOCOL_H
