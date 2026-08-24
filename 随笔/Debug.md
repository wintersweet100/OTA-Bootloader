#### 一、我想初步在bootloader读取具有头部的app文件的基础上，写一个用bootloader读取写在0x08029000【app1文件】地址上的app文件的头部，然后将代码复制到0x08009000【运行区】地址上，并且跳转运行代码。

在此过程中接连遇到两个问题：

1.bootloader读取app文件头部，读取错误。

原因：app文件烧录位置烧录错误。应该烧录在0x08029000处，因为之前的测试代码，就忘记更改烧录位置了，错误烧录在了0x08009000处。

![image-20251221230332223](C:\Users\Liuzihe\AppData\Roaming\Typora\typora-user-images\image-20251221230332223.png)

2.bootloader可以读取正确的app头部。读取效果见下图：

但是，并没有执行对应的app程序代码。下图中的串口打印信息仅仅是bootloader的执行代码现象。于是，我怀疑是文件过大，bootloader并没有成功将代码复制过去，我便进入调试模式查看0x08009000处的字节是否与app.bin的一致。

0x08009000处并没有任何数据，于是我反应过来。flash空间并不支持这样软件进行数据复制，来更新(烧录)flash。之前，我用这个bootloader迁移代码/app自我复制代码的目标地址都是RAM中的空间，RAM是支持软件更改其中的数据的。

![image-20251221230531668](C:\Users\Liuzihe\AppData\Roaming\Typora\typora-user-images\image-20251221230531668.png)



![IMG_20251223_110013](D:\Software City\QQ\IMG_20251223_110013.jpg)



```
>>> 准备发送: APP1_Freertos.bin
>>> 启动 Z-modem 传输...

[sz错误] Sending: APP1_Freertos.bin

[sz错误] 

[sz错误] 
Transfer complete


--- 传输成功 ---
Board Transfer complete
Received 0x000042c4 bytes.
Saved at ram address 0x20000000
msh >
```

**2.网络调试助手(PC端)->ESP-01s(Server)->串口1调试助手
发送过来的数据后面跟着：

```
md rz help md rz helpmd rz help 

SR:APMAC,"4e:75:25:33:4d:27"

OK

md rz help md rz helpmd rz help 

SR:APMAC,"4e:75:25:33:4d:27"

OK
```

需要取消掉。	





下面是我正在进行的一个收发.bin文件的测试。
QT上位机端—Wifi（ESP8266）—-单片机串口3
QT上位机呢将发送一个.bin文件到单片机，发送过程中会有收发交互的确认信息。但是还在第一步，Qt刚给单片机发送一个以0xAA为帧头的数据，单片机按规定是要回复FTP_READY的，但是在这个阶段出问题了。
单片机端调试信息为：AT+CIPSEND=0,110
AT+CIPSEND=0,127
AT+CIPSEND=0,128
AT+CIPSEND=0,128
AT+AT+CIPSEND=0,128
AT+CIPSEND=0,128
AT+CIPSEND=0,128
AT+CIPSEND=0,128
AT+CIPSEND=0,128
AT+CIPSEND=0,128
AT+CIPSEND=0,128
ATAT+CIPSEND=0,128
AT+CIPSEND=0,128
AT+CIPSEND=0,128
AT+CIPSEND=0,128
AT+CIPSEND=0,128
AT+CIPSEND=0,128
AT+CIPSEND=0,128
AT
QT端调试信息为：
[FileTransfer] State changed to: 1
[FileTransfer] Sent FILE_START, Waiting for READY...
[FileTransfer] Received: 7 bytes "41542b43495053"
[FileTransfer] Failed to parse frame
[FileTransfer] Timeout!
[FileTransfer] Retrying... ( 1 / 3 )
[FileTransfer] Timeout!
[FileTransfer] Retrying... ( 2 / 3 )
[FileTransfer] Timeout!
[FileTransfer] Max retries exceeded
[FileTransfer] State changed to: 7

相关的代码为：
单片机代码：
static uint8_t  wifi_frame_buf[1024];  // WiFi帧缓冲区
static uint16_t wifi_frame_pos = 0;    // 缓冲区位置
void USART3_IRQHandler(void)
{
    uint8_t ucCh;
    USART_TypeDef *usart3 = USART3;
    // 状态机变量：仅在 g_WiFi_Shell_Enable = 1 时生效
    static uint8_t  state = 0;       // 0:找+IPD, 1:解析长度, 2:接收纯数据
    static uint8_t  match_idx = 0;
    static uint32_t rem_len = 0;     // 记录当前 IPD 包剩余需要接收的长度
    // 1. 处理接收中断 (RXNE)
    if (usart3->SR & UART_FLAG_RXNE)
    {
        ucCh = (uint8_t)(usart3->DR & 0x00FF);
        // --- 原始记录：无论什么模式，始终记录到系统缓冲区，用于掉线检测 ---
        if (strEsp8266_Fram_Record.InfBit.FramLength < (RX_BUF_MAX_LEN - 1))
        {
            strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength++] = ucCh;
        }
        // --- 分模逻辑：解决初始化与数据传输的冲突 ---
        if (g_WiFi_Shell_Enable == 0)
        {
            // 【模式 A：初始化模式】所有数据（OK, ready, AT）全部直接进入 rx_buf [3]
            // 这确保了初始化程序能读到 AT 指令响应，不会卡死
            ring_buffer_write(ucCh, &rx_buf);
        }
        else
        {
            // 【模式 B：Shell/数据传输模式】开启状态机剥离 +IPD 头部 [1]
            switch (state)
            {
                case 0: // 搜索 "+IPD" 头部
                    if (ucCh == "+IPD"[match_idx]) {
                        match_idx++;
                        if (match_idx == 4) {
                            state = 1;      // 匹配成功
                            match_idx = 0;
                            rem_len = 0;    // 准备解析长度
wifi_frame_pos = 0;  // 重置帧缓冲区
                        }
                    } else {
                        // 如果不是 +IPD，判断是否为回显（可选：非 IPD 数据也可根据需求写入 rx_buf）
                        match_idx = (ucCh == '+') ? 1 : 0;
                    }
                    break;
                case 1: // 解析长度字段（格式如：,0,1024:）
                    if (ucCh >= '0' && ucCh <= '9') {
                        rem_len = rem_len * 10 + (ucCh - '0');
                    } else if (ucCh == ':') {
                        if (rem_len > 0){
state = 2; // 解析到冒号，且长度有效，进入提取模式
                        }else {
state = 0;             // 长度无效，复位
match_idx = 0;
}
                    } else if (ucCh == ',') {
                        rem_len = 0; // 遇到逗号重置长度累加（跳过前面的 ID 字段）
                    }
                    break;
                case 2:
                // 提取纯净数据只写入帧缓冲区，不写入 rx_buf
                    if (wifi_frame_pos < sizeof(wifi_frame_buf)) {
                        wifi_frame_buf[wifi_frame_pos++] = ucCh;
                    }
                    rem_len--;
if(rem_len == 0){//一个完整的帧已接收
// 检测是否是文件传输协议帧（帧头 0xAA）
                        if (wifi_frame_pos >= 2 && wifi_frame_buf[0] == 0xAA) {
                            // 文件传输帧，调用文件传输处理函数
                            fileReceiverHandleData(&fileReceiver, wifi_frame_buf, wifi_frame_pos);
                        } else {
                            // 普通数据，写入 rx_buf（shell命令）
                            for (uint16_t i = 0; i < wifi_frame_pos; i++) {
                                ring_buffer_write(wifi_frame_buf[i], &rx_buf);
                            }
                        }
                        state = 0; // 重点：一旦接收完指定长度，立即回到搜索模式，剥离下一个包的头部
                        match_idx = 0;
                        wifi_frame_pos = 0;
}
                    break;
    default:
                    state = 0;
                    match_idx = 0;
                    rem_len = 0;
                    wifi_frame_pos = 0;
                    break;
            }
        }
    }
    // 2. 处理空闲中断 (IDLE)
    if (usart3->SR & UART_FLAG_IDLE)
    {
        // 清除 IDLE 标志 [1]
        __HAL_UART_CLEAR_IDLEFLAG(&huart_esp8266);
        strEsp8266_Fram_Record.InfBit.FramFinishFlag = 1;
        strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength] = '\0';
        // 检测 TCP 链接是否关闭
        if (strstr((char*)strEsp8266_Fram_Record.Data_RX_BUF, "CLOSED\r\n")) {
            ucTcpClosedFlag = 1;
        }
        /*
         * 注意：不要在这里执行 strEsp8266_Fram_Record.InfBit.FramLength = 0;
         * 长度清零应由 main 函数中处理完 AT 指令响应后手动执行，
         * 否则初始化程序会因为数据被提前清空而认为没收到 "OK" [3]。
         */
        // 传输阶段非数据接收态时，重置状态机
        if (g_WiFi_Shell_Enable != 0 && state != 2) {
            state = 0;
            match_idx = 0;
rem_len = 0;          
            wifi_frame_pos = 0;                          
        }
    }
}
void fileReceiverHandleData(FileReceiver _receiver, const uint8_t_ data, uint16_t len) {
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
// 单独提取帧处理逻辑
void processFrame(FileReceiver _receiver, Frame_ frame) {
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
//            if (receiver->receivedBytes % 102400 0 || receiver->receivedBytes receiver->fileSize) {
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
ESP8266_SendString(DISABLE,(char * )txBuffer,txLen,Multiple_ID_0);
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
ESP8266_SendString(DISABLE,(char * )txBuffer,txLen,Multiple_ID_0);
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
ESP8266_SendString(DISABLE,(char * )txBuffer,txLen,Multiple_ID_0);
}
}
bool ESP8266_SendString ( FunctionalState enumEnUnvarnishTx, char * pStr, u32 ulStrLength, ENUM_ID_NO_TypeDef ucId )
{
char cStr [20];
bool bRet = false;
if ( enumEnUnvarnishTx )
{
macESP8266_Usart ( "%s", pStr );
bRet = true;
}
else
{
if ( ucId < 5 )
sprintf ( cStr, "AT+CIPSEND=%d,%d", ucId, ulStrLength );
else
sprintf ( cStr, "AT+CIPSEND=%d", ulStrLength );
strEsp8266_Fram_Record.InfBit.FramLength = 0;
strEsp8266_Fram_Record.InfBit.FramFinishFlag = 0;
        // 发送 AT 指令
        macESP8266_Usart("%s\r\n", cStr);
        // 等待 '>' 提示符（表示可以发送数据了）
uint32_t timeout = 500;//最大500ms超时时间
while(timeout > 0 && strstr((char*)strEsp8266_Fram_Record.Data_RX_BUF, ">") == NULL){
Delay_ms(10);
timeout -= 10;
}
if(timeout <= 0 ){
printf("[Esp8266] Timeout for waiting '>'\r\n");
return false;
}
        // 第2步：发送真实数据（不要用 ESP8266_Cmd，不要自动加 \r\n）
        strEsp8266_Fram_Record.InfBit.FramLength = 0;
        strEsp8266_Fram_Record.InfBit.FramFinishFlag = 0;
        //  直接发送原始数据，不加任何额外字符
        HAL_UART_Transmit(&huart_esp8266, (uint8_t*)pStr, ulStrLength, 500);
        // 等待 SEND OK
        timeout = 1000;  // 1000ms 超时
        while (timeout > 0 && strstr((char*)strEsp8266_Fram_Record.Data_RX_BUF, "SEND OK") == NULL) {
            Delay_ms(10);
            timeout -= 10;
        }
        if (timeout <= 0) {
            printf("[ESP8266] Timeout waiting for 'SEND OK'\r\n");
            return false;
        }
        bRet = true;
  }
return bRet;
}
QT端相关代码为：

```
void BootloaderPage::onSendBinClicked()
```

```
{
```

```
    // 检查是否有可用的TCP连接（WiFi或以太网）
```

```
    QTcpSocket *activeSocket = nullptr;
```

```
    QString channelName;
```

```
    if (useWiFi->isChecked() && wifiSocket && wifiSocket->state() == QTcpSocket::ConnectedState) {
```

```
        activeSocket = wifiSocket;
```

```
        channelName = u8"WiFi";
```

```
    } else if (useEthernet->isChecked() && ethernetSocket && ethernetSocket->state() == QTcpSocket::ConnectedState) {
```

```
        activeSocket = ethernetSocket;
```

```
        channelName = u8"以太网";
```

```
    } else if (useSerial->isChecked() && parentWidget) {
```

```
        QSerialPort *serialPort = parentWidget->getSerialPort();
```

```
        if (serialPort && serialPort->isOpen()) {
```

```
            // 复用Widget的串口发送.bin功能
```

```
            parentWidget->triggerSendBin();
```

```
            return;
```

```
        }
```

```
    }
```

```
    if (!activeSocket) {
```

```
        QMessageBox::warning(this, u8"提示", u8"请先建立TCP连接（WiFi或以太网）");
```

```
        return;
```

```
    }
```

```
    if (szProcess && szProcess->state() == QProcess::Running) {
```

```
        QMessageBox::warning(this, u8"提示", u8"传输正在进行中");
```

```
        return;
```

```
    }
```

```
    if (fileTransferManager) {
```

```
        QMessageBox::warning(this, u8"提示", u8"传输正在进行中");
```

```
        return;
```

```
    }
```

```
    // 选择文件
```

```
    QString filePath = QFileDialog::getOpenFileName(this, u8"选择.bin文件", "", "*.bin");
```

```
    if (filePath.isEmpty())
```

```
        return;
```

```
    pendingFilePath = filePath;
```

```
    receiveArea->appendPlainText(u8"\n>>> 准备通过" + channelName + u8"发送: " + QFileInfo(filePath).fileName());
```

```
    // 先发送rz命令
```

```
//    receiveArea->appendPlainText(u8"请在目标设备执行 'rz' 命令...\n");
```

```
//    QString rzCmd = "rz\n";
```

```
//    QByteArray rzBytes = rzCmd.toLocal8Bit();
```

```
//    activeSocket->write(rzBytes);
```

```
//    activeSocket->flush();
```

```
//    // 等待2秒后启动传输，等着的2s是等stm32端的rz恢复ACK，等待接收
```

```
//    QTimer::singleShot(2000, this, &BootloaderPage::startZmodemTransfer);
```

```
//    //创建传输管理器前，断开界面的readyRead连接
```

```
    if(activeSocket == wifiSocket){
```

```
        disconnect(wifiSocket,&QTcpSocket::readyRead,this,&BootloaderPage::onWiFiReadyRead);
```

```
    }
```

```
    else{
```

```
        disconnect(ethernetSocket,&QTcpSocket::readyRead,this,&BootloaderPage::onEthernetReadyRead);
```

```
    }
```

```
    // 创建传输管理器
```

```
    fileTransferManager = new FileTransferManager(activeSocket, this);
```

```
    connect(fileTransferManager, &FileTransferManager::transferStarted,
```

```
            this, [this](const QString &fileName, uint32_t fileSize) {
```

```
        receiveArea->appendPlainText(QString(u8"[传输] 开始发送: %1 (%2 字节)")
```

```
                                     .arg(fileName).arg(fileSize));
```

```
    });
```

```
    //FileTransferManager::onSocketReadyRead() 中收到设备的 READY 帧后--
```

```
    //"emit transferStarted(QFileInfo(file).fileName(), totalFileSize);"--触发
```

```
    //作用：向界面输出传输开始的提示信息，显示文件名和大小
```

```
    connect(fileTransferManager, &FileTransferManager::transferProgress,
```

```
            this, [this](uint32_t sent, uint32_t total) {
```

```
        receiveArea->appendPlainText(QString(u8"[传输] 进度: %1 / %2 字节")
```

```
                                     .arg(sent).arg(total));
```

```
    });
```

```
    //每次收到设备的 ACK 帧后（表示一块数据已成功接收）
```

```
    //emit transferProgress(sentBytes, totalFileSize);--触发
```

```
    //作用：实时显示传输进度
```

```
    connect(fileTransferManager, &FileTransferManager::transferFinished,
```

```
            this, [this](bool , const QString &message) {
```

```
        receiveArea->appendPlainText(u8"[传输] " + message);
```

```
        fileTransferManager->deleteLater();
```

```
        fileTransferManager = nullptr;
```

```
    });
```

```
    //所有数据块发送完毕，发送 FILE_END 帧后--触发
```

```
    //deleteLater() - 延迟删除 FileTransferManager 对象（安全释放资源）
```

```
    if(activeSocket == wifiSocket){
```

```
        connect(wifiSocket,&QTcpSocket::readyRead,this,&BootloaderPage::onWiFiReadyRead);
```

```
    }
```

```
    else{
```

```
        connect(ethernetSocket,&QTcpSocket::readyRead,this,&BootloaderPage::onEthernetReadyRead);
```

```
    }
```

```
    connect(fileTransferManager, &FileTransferManager::error,
```

```
            this, [this](const QString &errMsg) {
```

```
        receiveArea->appendPlainText(u8"[传输错误] " + errMsg);
```

```
        fileTransferManager->deleteLater();
```

```
        fileTransferManager = nullptr;
```

```
    });
```

```
    //传输过程中出现错误时触发
```

```
    //清理资源
```

```
    if(activeSocket == wifiSocket){
```

```
        connect(wifiSocket,&QTcpSocket::readyRead,this,&BootloaderPage::onWiFiReadyRead);
```

```
    }
```

```
    else{
```

```
        connect(ethernetSocket,&QTcpSocket::readyRead,this,&BootloaderPage::onEthernetReadyRead);
```

```
    }
```

```
    fileTransferManager->sendFile(filePath);
```

```
}
```

```
FileTransferManager::FileTransferManager(QTcpSocket *socket, QObject *parent)
```

```
    : QObject(parent), socket(socket), state(IDLE), totalFileSize(0),
```

```
      sentBytes(0), currentBlockSize(0), retryCount(0) {
```

```
    // 初始化成员变量：socket指针、状态为空闲、文件大小/已发送字节/块大小/重试次数都为0
```

```
    timeoutTimer = new QTimer(this);//创建超时定时器，生命周期由this管理
```

```
    connect(timeoutTimer, &QTimer::timeout, this, &FileTransferManager::onTransferTimeout);
```

```
    // 定时器超时时调用onTransferTimeout()
```

```
    connect(socket, &QTcpSocket::readyRead, this, &FileTransferManager::onSocketReadyRead);
```

```
    // socket有数据可读时调用onSocketReadyRead()
```

```
}
```

```
void FileTransferManager::sendFile(const QString &filePath) {
```

```
    if (state != IDLE) {              // 检查是否已有传输在进行
```

```
        emit error(u8"传输已在进行中");
```

```
        return;
```

```
    }
```

```
    if (!socket || socket->state() != QTcpSocket::ConnectedState) {
```

```
        // 检查socket是否存在且已连接
```

```
        emit error(u8"TCP 连接未建立");
```

```
        return;
```

```
    }
```

```
    file.setFileName(filePath);                 // 设置要发送的文件路径
```

```
    if (!file.open(QIODevice::ReadOnly)) {      // 以只读模式打开
```

```
        emit error(u8"无法打开文件: " + filePath);
```

```
        return;
```

```
    }
```

```
    totalFileSize = file.size();                // 获取文件总大小
```

```
    sentBytes = 0;                              // 已发送字节数重置为0
```

```
    retryCount = 0;                             // 重试次数重置为0
```

```
    setState(WAITING_FOR_READY);                // 状态改为等待设备准备好
```

```
    //主动向单片机端发送文件头，触发单片机响应
```

```
    QByteArray header = FileTransferProtocol::createFileHeader(
```

```
                    QFileInfo(file).fileName(),totalFileSize);
```

```
    QByteArray frameData = FileTransferProtocol::createFrame(
```

```
                    FileTransferProtocol::READY,header);
```

```
    socket->write(frameData);
```

```
    socket->flush();
```

```
    qDebug() << "[FileTransfer] Sent FILE_START, Waiting for READY...";
```

```
    timeoutTimer->start(TIMEOUT_MS);            // 启动5秒超时定时器
```

```
    // ★ 添加：主动发送一个测试帧，看单片机能否收到----------能够收到并反应
```

```
//    QByteArray testFrame = FileTransferProtocol::createFrame(
```

```
//        FileTransferProtocol::READY, QByteArray("TEST"));
```

```
//    socket->write(testFrame);
```

```
//    socket->flush();
```

```
//    qDebug() << "[FileTransfer] Sent test frame:" << testFrame.toHex();
```

```
}
```

```
//接收数据处理
```

```
void FileTransferManager::onSocketReadyRead() {
```

```
    QByteArray rawData = socket->readAll();     // 读取socket中所有数据
```

```
    qDebug() << "[FileTransfer] Received:" << rawData.size() << "bytes" << rawData.toHex();
```

```
    // 打印接收的字节数和十六进制内容
```

```
//    // ★ 过滤掉 AT 指令相关的数据
```

```
//    QByteArray filteredData;
```

```
//    for (int i = 0; i < rawData.size(); i++) {
```

```
//        uint8_t byte = (uint8_t)rawData[i];
```

```
//        // ★ 检查是否是文件传输帧头 0xAA
```

```
//        if (byte == 0xAA) {
```

```
//            // 从这个位置开始收集帧数据
```

```
//            filteredData.append(rawData.mid(i));
```

```
//            break;
```

```
//        }
```

```
//    }
```

```
//    // 如果没有找到帧头，说明全是 AT 指令，直接丢弃
```

```
//    if (filteredData.isEmpty()) {
```

```
//        qDebug() << "[FileTransfer] No frame header found, discarding AT command data";
```

```
//        return;
```

```
//    }
```

```
//    qDebug() << "[FileTransfer] Filtered data:" << filteredData.toHex();
```

```
    FileTransferProtocol::Frame frame;
```

```
    if (!FileTransferProtocol::parseFrame(rawData, frame)) {
```

```
        // 尝试解析帧，失败则返回
```

```
        qDebug() << "[FileTransfer] Failed to parse frame";
```

```
        return;
```

```
    }
```

```
    timeoutTimer->stop();                       // 收到数据，停止超时定时器
```

```
    switch (frame.type) {                       // 根据帧类型处理
```

```
        case FileTransferProtocol::READY:       // 设备准备好
```

```
            if (state == WAITING_FOR_READY) {
```

```
                qDebug() << "[FileTransfer] Device ready, sending file header...";
```

```
                // 创建文件头（文件名+大小）
```

```
                QByteArray header = FileTransferProtocol::createFileHeader(
```

```
                    QFileInfo(file).fileName(), totalFileSize);
```

```
                // 将文件头封装成FILE_START帧
```

```
                QByteArray frameData = FileTransferProtocol::createFrame(
```

```
                    FileTransferProtocol::FILE_START, header);
```

```
                socket->write(frameData);       // 发送帧
```

```
                socket->flush();                // 立即刷新缓冲区
```

```
                setState(SENDING_FILE_DATA);    // 状态改为发送数据
```

```
                emit transferStarted(QFileInfo(file).fileName(), totalFileSize);
```

```
                // 发出传输开始信号
```

```
                sendNextBlock();                // 发送第一块数据
```

```
            }
```

```
            break;
```

```
//        case FileTransferProtocol::ACK:         // 接收方确认
```

```
//            handleAck();
```

```
//            break;
```

```
        case FileTransferProtocol::ACK:
```

```
            if (state == WAITING_FOR_READY) {
```

```
                qDebug() << "[FileTransfer] Device ready, sending file data...";
```

```
                setState(SENDING_FILE_DATA);
```

```
                emit transferStarted(QFileInfo(file).fileName(), totalFileSize);
```

```
                sendNextBlock();
```

```
            } else if (state == SENDING_FILE_DATA) {
```

```
                handleAck();
```

```
            }
```

```
            break;
```

```
        case FileTransferProtocol::NAK:         // 接收方否认（需要重传）
```

```
            handleNak();
```

```
            break;
```

```
        case FileTransferProtocol::ERROR:       // 接收方报错
```

```
            handleError();
```

```
            break;
```

```
        default:
```

```
            qDebug() << "[FileTransfer] Unknown frame type:" << frame.type;
```

```
    }
```

```
}
```
