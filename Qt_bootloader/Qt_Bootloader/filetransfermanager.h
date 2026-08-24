#ifndef FILETRANSFERMANAGER_H
#define FILETRANSFERMANAGER_H

#include <QWidget>
#include <QObject>
#include <QTcpSocket>
#include <QFile>
#include <QTimer>
#include "filetransferprotocol.h"

class FileTransferManager : public QObject
{
    Q_OBJECT
public:
    explicit FileTransferManager(QTcpSocket *socket, QObject *parent = nullptr);

    // 发送文件
    void sendFile(const QString &filePath);

    // 停止传输
    void stopTransfer();

signals:
    void transferStarted(const QString &fileName, uint32_t fileSize);
    void transferProgress(uint32_t sentBytes, uint32_t totalBytes);
    void transferFinished(bool success, const QString &message);
    void error(const QString &errorMsg);

private slots:
    void onSocketReadyRead();
    void onTransferTimeout();

private:
    enum TransferState {
        IDLE,
        WAITING_FOR_READY,
        SENDING_FILE_START,
        SENDING_FILE_DATA,
        SENDING_FILE_END,
        WAITING_FOR_ACK,
        TRANSFER_COMPLETE,
        TRANSFER_ERROR
    };

    void setState(TransferState newState);
    void sendNextBlock();
    void handleAck();
    void handleNak();
    void handleError();
    void cleanup();

    QTcpSocket *socket;
    QFile file;
    QTimer *timeoutTimer;

    TransferState state;
    uint32_t totalFileSize;
    uint32_t sentBytes;
    uint32_t currentBlockSize;
    int retryCount;

    static const int BLOCK_SIZE = 1024;
    static const int TIMEOUT_MS = 300;
    static const int MAX_RETRIES = 3;

};

#endif // FILETRANSFERMANAGER_H
