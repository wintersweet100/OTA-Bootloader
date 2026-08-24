#include "filetransfermanager.h"
#include <QFileInfo>
#include <QDebug>
#include <QObject>

FileTransferManager::FileTransferManager(QTcpSocket *socket, QObject *parent)
    : QObject(parent), socket(socket), state(IDLE), totalFileSize(0),
      sentBytes(0), currentBlockSize(0), retryCount(0) {
    // 初始化成员变量：socket指针、状态为空闲、文件大小/已发送字节/块大小/重试次数都为0

    timeoutTimer = new QTimer(this);//创建超时定时器，生命周期由this管理
    connect(timeoutTimer, &QTimer::timeout, this, &FileTransferManager::onTransferTimeout);
    // 定时器超时时调用onTransferTimeout()

    connect(socket, &QTcpSocket::readyRead, this, &FileTransferManager::onSocketReadyRead);
    // socket有数据可读时调用onSocketReadyRead()
}

//发送文件
void FileTransferManager::sendFile(const QString &filePath) {
    if (state != IDLE) {              // 检查是否已有传输在进行
        emit error(u8"传输已在进行中");
        return;
    }

    if (!socket || socket->state() != QTcpSocket::ConnectedState) {
        // 检查socket是否存在且已连接
        emit error(u8"TCP 连接未建立");
        return;
    }

    file.setFileName(filePath);                 // 设置要发送的文件路径
    if (!file.open(QIODevice::ReadOnly)) {      // 以只读模式打开
        emit error(u8"无法打开文件: " + filePath);
        return;
    }

    totalFileSize = file.size();                // 获取文件总大小
    sentBytes = 0;                              // 已发送字节数重置为0
    retryCount = 0;                             // 重试次数重置为0
    setState(WAITING_FOR_READY);                // 状态改为等待设备准备好

    //主动向单片机端发送文件头，触发单片机响应
    QByteArray header = FileTransferProtocol::createFileHeader(
                    QFileInfo(file).fileName(),totalFileSize);
    QByteArray frameData = FileTransferProtocol::createFrame(
                    FileTransferProtocol::READY,header);
    socket->write(frameData);
    socket->flush();

    qDebug() << "[FileTransfer] Sent FILE_START, Waiting for READY...";

    timeoutTimer->start(TIMEOUT_MS);            // 启动5秒超时定时器

    // ★ 添加：主动发送一个测试帧，看单片机能否收到----------能够收到并反应
//    QByteArray testFrame = FileTransferProtocol::createFrame(
//        FileTransferProtocol::READY, QByteArray("TEST"));
//    socket->write(testFrame);
//    socket->flush();
//    qDebug() << "[FileTransfer] Sent test frame:" << testFrame.toHex();

}

//停止传输
void FileTransferManager::stopTransfer() {
    if (state != IDLE) {                        // 如果正在传输
        setState(TRANSFER_ERROR);               // 设置为错误状态
        cleanup();                              // 清理资源
    }
}

//接收数据处理
void FileTransferManager::onSocketReadyRead() {
    QByteArray rawData = socket->readAll();     // 读取socket中所有数据
    qDebug() << "[FileTransfer] Received:" << rawData.size() << "bytes" << rawData.toHex();
    // 打印接收的字节数和十六进制内容

    // ★ 打印 ASCII 可见字符
    QString asciiStr;
    for (int i = 0; i < rawData.size(); i++) {
        uint8_t byte = (uint8_t)rawData[i];
        if (byte >= 0x20 && byte < 0x7F) {
            asciiStr += (char)byte;
        } else {
            asciiStr += QString("[%1]").arg(byte, 2, 16, QChar('0'));
        }
    }
    qDebug() << "[FileTransfer] ASCII:" << asciiStr;

    // ★ 逐字节打印
    qDebug() << "[FileTransfer] Byte-by-byte:";
    for (int i = 0; i < rawData.size(); i++) {
        uint8_t byte = (uint8_t)rawData[i];
        qDebug().noquote() << QString("  [%1] = 0x%2 (%3)")
            .arg(i, 3)
            .arg(byte, 2, 16, QChar('0'))
            .arg(byte >= 0x20 && byte < 0x7F ? (char)byte : '?');
    }

    // ★ 打印缓冲区状态
    qDebug() << "[FileTransfer] Socket state:" << socket->state();
    qDebug() << "[FileTransfer] Bytes available:" << socket->bytesAvailable();


    FileTransferProtocol::Frame frame;
    if (!FileTransferProtocol::parseFrame(rawData, frame)) {
        // 尝试解析帧，失败则返回
        qDebug() << "[FileTransfer] Failed to parse frame";
        return;
    }

    timeoutTimer->stop();                       // 收到数据，停止超时定时器

    switch (frame.type) {                       // 根据帧类型处理
        case FileTransferProtocol::READY:       // 设备准备好
            if (state == WAITING_FOR_READY) {
                qDebug() << "[FileTransfer] Device ready, sending file header...";

                // 创建文件头（文件名+大小）
                QByteArray header = FileTransferProtocol::createFileHeader(
                    QFileInfo(file).fileName(), totalFileSize);
                // 将文件头封装成FILE_START帧
                QByteArray frameData = FileTransferProtocol::createFrame(
                    FileTransferProtocol::FILE_START, header);
                socket->write(frameData);       // 发送帧
                socket->flush();                // 立即刷新缓冲区
                setState(SENDING_FILE_DATA);    // 状态改为发送数据
                emit transferStarted(QFileInfo(file).fileName(), totalFileSize);
                // 发出传输开始信号
                sendNextBlock();                // 发送第一块数据
            }
            break;
//        case FileTransferProtocol::ACK:         // 接收方确认
//            handleAck();
//            break;
        case FileTransferProtocol::ACK:
            if (state == WAITING_FOR_READY) {
                qDebug() << "[FileTransfer] Device ready, sending file data...";
                setState(SENDING_FILE_DATA);
                emit transferStarted(QFileInfo(file).fileName(), totalFileSize);
                sendNextBlock();
            } else if (state == SENDING_FILE_DATA) {
                handleAck();
            }
            break;

        case FileTransferProtocol::NAK:         // 接收方否认（需要重传）
            handleNak();
            break;
        case FileTransferProtocol::ERROR:       // 接收方报错
            handleError();
            break;
        default:
            qDebug() << "[FileTransfer] Unknown frame type:" << frame.type;
    }
}

//超时处理
void FileTransferManager::onTransferTimeout() {
    qDebug() << "[FileTransfer] Timeout!";

    retryCount++;
    if (retryCount >= MAX_RETRIES) {
        qDebug() << "[FileTransfer] Max retries exceeded";
        emit error(u8"传输超时");
        setState(TRANSFER_ERROR);
        cleanup();
        return;
    }

    qDebug() << "[FileTransfer] Retrying... (" << retryCount << "/" << MAX_RETRIES << ")";

    // ★ 重新发送当前块
    if (state == SENDING_FILE_DATA) {
        // 回退文件指针
//        file.seek(sentBytes);
        sendNextBlock();
    }else if (state == WAITING_FOR_READY) {
        // 重新发送 FILE_START
        QByteArray header = FileTransferProtocol::createFileHeader(
            QFileInfo(file).fileName(), totalFileSize);
        QByteArray frameData = FileTransferProtocol::createFrame(
            FileTransferProtocol::FILE_START, header);
        socket->write(frameData);
        socket->flush();
        timeoutTimer->start(TIMEOUT_MS);
    }
//    if (retryCount < MAX_RETRIES) {             // 重试次数未超过3次
//        retryCount++;
//        qDebug() << "[FileTransfer] Retrying... (" << retryCount << "/" << MAX_RETRIES << ")";
//        if (state == WAITING_FOR_READY) {
//            timeoutTimer->start(TIMEOUT_MS);    // 继续等待READY，重启定时器
//        } else if (state == SENDING_FILE_DATA) {
//            sendNextBlock();                    // 重新发送当前块
//        }
//    } else {
//        emit error(u8"传输超时");               // 超过3次重试，报错
//        setState(TRANSFER_ERROR);
//        cleanup();
//    }
}

//发送数据块
void FileTransferManager::sendNextBlock() {

    qDebug() << "[FileTransfer] sendNextBlock called";
    qDebug() << "[FileTransfer] file.isOpen()=" << file.isOpen();
    qDebug() << "[FileTransfer] file.pos()=" << file.pos();
    qDebug() << "[FileTransfer] file.size()=" << file.size();
    qDebug() << "[FileTransfer] sentBytes=" << sentBytes << "totalFileSize=" << totalFileSize;


    if (sentBytes >= totalFileSize) {        //所有数据已发送
        qDebug() << "[FileTransfer] Sending FILE_END...";  // 调试信息

        // 发送文件结束帧
        QByteArray frameData = FileTransferProtocol::createFrame(
            FileTransferProtocol::FILE_END, QByteArray());
        socket->write(frameData);
        socket->flush();

        qDebug() << "[FileTransfer] FILE_END sent:" << frameData.toHex();

        setState(TRANSFER_COMPLETE);        //状态改为完成
        emit transferFinished(true, u8"文件传输成功");
        cleanup();
        return;
    }

    if(!file.isOpen()){
        qDebug() << "[FileTransfer] ERROR:File is not open!";
        emit error(u8"文件已关闭");
        return;
    }

    // 定位到下一个数据块的起始位置
    file.seek(sentBytes);

    uint32_t remainingSize = totalFileSize - sentBytes;
    uint32_t blockSize = (remainingSize>BLOCK_SIZE)?BLOCK_SIZE:remainingSize;

    qDebug()<<"[FileTransfer] Attempting to read"<< blockSize << "bytes";

    QByteArray block = file.read(blockSize);//读取文件

    qDebug()<<"[FileTransfer] Actually read"<< block.size()<<"bytes";
    qDebug()<<"[FileTransfer] file.error()="<<file.error();

    // 读取数据块
    //QByteArray block = file.read(BLOCK_SIZE);  //读取4KB数据库
    if (block.isEmpty()) {                     //如果读取失败
        qDebug()<<"[FileTransfer] Block is empty!errorString"<<file.errorString();
        emit error(u8"读取文件失败："+ file.errorString());
        setState(TRANSFER_ERROR);
        cleanup();
        return;
    }

    currentBlockSize = block.size();           //记录本块大小

    //  将数据块封装成FILE_DATA帧
    QByteArray frameData = FileTransferProtocol::createFrame(
        FileTransferProtocol::FILE_DATA, block);
    qDebug()<<"[FileTransfer] Created frame,size"<< frameData.size();

    qint64 written = socket->write(frameData);                 //发送帧
    socket->flush();                          //立即刷新

    //qDebug() << "[FileTransfer] Sent block:" << sentBytes << "/" << totalFileSize;
    qDebug() << "[FileTransfer] Sent block:" << sentBytes << "-"
             << (sentBytes + currentBlockSize) << "/" << totalFileSize
             << "(" << written << "bytes written)";
    retryCount = 0;                           //重试计数器重置

    timeoutTimer->start(TIMEOUT_MS);          //启动5秒超时等待ACK
}

//处理确认
void FileTransferManager::handleAck() {
    if (state == SENDING_FILE_DATA) {         // 确保在发送数据状态
        sentBytes += currentBlockSize;        // 累加已发送字节数

        qDebug() << "[FileTransfer] ACK: currentBlockSize=" << currentBlockSize
                 << "sentBytes=" << sentBytes << "totalFileSize=" << totalFileSize;
        emit transferProgress(sentBytes, totalFileSize);
        // 发出进度信号

        if (sentBytes >= totalFileSize) {
            qDebug() << "[FileTransfer] All data sent, sending FILE_END...";
        }

        sendNextBlock();                      // 发送下一块
    }
}

//处理否认(重传)
void FileTransferManager::handleNak() {
    if (state == SENDING_FILE_DATA) {
        if (retryCount < MAX_RETRIES) {       // 重试次数未超过3次
            retryCount++;
            qDebug() << "[FileTransfer] NAK received, retrying...";

            file.seek(sentBytes);             // 文件指针回到已发送位置
            sendNextBlock();                  // 重新发送当前块
        } else {
            emit error(u8"重传次数过多");       // 超过3次重试，报错
            setState(TRANSFER_ERROR);
            cleanup();
        }
    }
}

//处理设备错误
void FileTransferManager::handleError() {
    emit error(u8"设备报告错误");               // 发出错误信号
    setState(TRANSFER_ERROR);                 // 状态改为错误
    cleanup();                                // 清理资源
}

//状态转换
void FileTransferManager::setState(TransferState newState) {
    state = newState;                         // 更新状态
    qDebug() << "[FileTransfer] State changed to:" << newState;
    // 打印状态变化日志
}

//资源清理
void FileTransferManager::cleanup() {
    timeoutTimer->stop();                       // 停止定时器
    if (file.isOpen()) {
        file.close();                           // 关闭文件
    }
    state = IDLE;                               // 状态恢复为空闲
}
