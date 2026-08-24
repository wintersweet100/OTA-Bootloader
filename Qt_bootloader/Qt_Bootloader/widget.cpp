#include "widget.h"
#include "bootloaderpage.h"

void Widget::ReceiveAreaInit(void) {
    receivedArea = new QPlainTextEdit(serialPage);
    receivedArea->setGeometry(30,20,800,400);
    receivedArea->setReadOnly(true);

    QPushButton *clearReceiveArea = new QPushButton(u8"清空接收区", serialPage);
    clearReceiveArea->setGeometry(680, 430,150, 50);
    connect(clearReceiveArea, &QPushButton::clicked, [=]() {
        receivedArea->clear();
    }); // 槽连接
}

void Widget::SendAreaInit(void) {
    sendArea = new QPlainTextEdit(serialPage);
    sendArea->setGeometry(30, 500,800, 100);

    // --- 新增：发送新行 复选框 ---
    addNewLine = new QCheckBox(u8"发送新行", serialPage);
    addNewLine->setGeometry(30,630,100,30);
    addNewLine->setChecked(true); // 默认勾选

    sendButton = new QPushButton(u8"发送", serialPage);
    sendButton->setGeometry(500, 630,150, 50);
    sendButton->setDisabled(true);

    connect(sendButton, &QPushButton::clicked, [=]() {
        QString data = sendArea->toPlainText();
        if (data.isEmpty() && !addNewLine->isChecked())
            return; // 只有在既没数据也不发新行时才返回

        QByteArray finalBytes; // 定义一个唯一的缓冲区
        /*根据发送模式选择发送不同格式的数据*/
        if (sendMode->currentText() == "HEX") {

            for (int i = 0; i < data.size(); ++i) {
                if (data[i] == ' ')
                    continue;                                 // 跳过空格
                int num = data.mid(i, 2).toUInt(nullptr, 16); // 取两个字符转为16进制数
                i++;                                          // 跳过已处理的第二个字符
                finalBytes.append((char)num);
            }
            // 如果选中了发送新行, HEX 模式也要发送新行，通常是追加 0D 0A
            if (addNewLine->isChecked()) {
                finalBytes.append('\n');
            }
        } else {
            finalBytes = data.toLocal8Bit();
            // 如果选中了发送新行
            if (addNewLine->isChecked()) {
                if (!finalBytes.endsWith(' ')) { // 强行补丁:避免help <>打印命令语法时出现中间空行现象。
                    finalBytes.append(' ');
                }
                finalBytes.append("\n"); // 追加回车换行
            }
        }
        serialPort->write(finalBytes);
    });

    QPushButton *clearSendArea = new QPushButton(u8"清空发送区", serialPage);
    clearSendArea->setGeometry(680, 630,150, 50);
    connect(clearSendArea, &QPushButton::clicked, [=]() {
        sendArea->clear();
    });

    sendBinBtn = new QPushButton(u8"发送.bin文件", serialPage);
    sendBinBtn->setGeometry(320,630,150,50);
    // 连接到你已经写好的槽函数
    connect(sendBinBtn, &QPushButton::clicked, this, &Widget::on_btnSelectAndSendBin_clicked);
    sendBinBtn->setDisabled(true);
}

void Widget::SetupInit() {

    // 跳到"网络调试"页面的按钮
    QPushButton *btnNetwork = new QPushButton(u8"网络调试页->", serialPage);
    btnNetwork->setGeometry(850,680,120, 35);
    connect(btnNetwork, &QPushButton::clicked, this, &Widget::switchToNetwork);

    this->portNumber = new QComboBox(serialPage);
    this->baudRate = new QComboBox(serialPage);
    this->dataSize = new QComboBox(serialPage);
    this->stopSize = new QComboBox(serialPage);
    this->check = new QComboBox(serialPage);
    this->receiveMode = new QComboBox(serialPage);
    this->sendMode = new QComboBox(serialPage);

    /*添加下拉框选项*/
    this->baudRate->addItems({"4800", "9600", "115200"});
    this->baudRate->setCurrentText("115200"); // 设置默认选中 115200
    this->dataSize->addItem("8");
    this->dataSize->setCurrentText("8"); // 设置默认8个数据位
    this->stopSize->addItems({"1", "1.5", "2"});
    this->stopSize->setCurrentText("1"); // 设置默认停止位为1位
    this->check->addItems({u8"无校验", u8"奇校验", u8"偶校验"});
    this->check->setCurrentText(u8"无校验");
    this->receiveMode->addItems({"HEX", u8"文本格式"});
    this->receiveMode->setCurrentText("文本格式");
    this->sendMode->addItems({"HEX", u8"文本格式"});
    this->sendMode->setCurrentText("文本格式");

    /*下拉框前标识文字*/
    QLabel *portlabel = new QLabel(u8"串口号", serialPage);
    QLabel *baudlabel = new QLabel(u8"波特率", serialPage);
    QLabel *datalabel = new QLabel(u8"数据位", serialPage);
    QLabel *stoplabel = new QLabel(u8"停止位", serialPage);
    QLabel *checklabel = new QLabel(u8"校验位", serialPage);
    QLabel *receivelabel = new QLabel(u8"接收格式", serialPage);
    QLabel *sendlabel = new QLabel(u8"发送格式", serialPage);

    /*容器,批量布局操作？*/
    QVector<QComboBox *> setups = {portNumber,baudRate,dataSize,stopSize,check,receiveMode,sendMode};
    QVector<QLabel *> labels = {portlabel,baudlabel,datalabel,stoplabel,checklabel,receivelabel,sendlabel};

    for (int i = 0; i < setups.size(); ++i) {
        setups[i]->setGeometry(950, 20 + i * 80,200,50);
        labels[i]->setGeometry(850, 40 + i * 80,200, 50);
    }
}

void Widget::beginUART() {
    Estbconnect = new QPushButton(u8"连接串口", serialPage);
    Estbconnect->setGeometry(850, 600,150, 50);

    Disconnect = new QPushButton(u8"断开串口", serialPage);
    Disconnect->setGeometry(1000, 600,150, 50);
    Disconnect->setDisabled(true);

    connect(Disconnect, &QPushButton::clicked, [=]() {
        sendButton->setDisabled(true);   // 断开连接,失能发送按钮
        Estbconnect->setDisabled(false); // 使能串口连接按钮
        Disconnect->setDisabled(true);   // 失能串口断开按钮
        sendBinBtn->setDisabled(true);   // 断开连接,失能发送按钮
        /*断开连接*/
        serialPort->close();
    });

    connect(Estbconnect, &QPushButton::clicked, [=]() {
        if (portNumber->currentText() != " ") {
            sendButton->setDisabled(false);
            Estbconnect->setDisabled(true);
            Disconnect->setDisabled(false);
            sendBinBtn->setDisabled(false);
            /*连接*/
            USART();
        } else {
            QMessageBox::critical(this, QString::fromLocal8Bit("串口打开失败"),
                                  QString::fromLocal8Bit("请确认串口是否连接正确"));
        }
    });
}

void Widget::SetupUiInit()
{
    // *********顶部标题和跳转按钮 **********************************************************
    QLabel *title = new QLabel(u8"串口调试助手", serialPage);
    title->setStyleSheet("font-size: 18px; font-weight: bold;");

    // 跳到"网络调试"页面的按钮
    QPushButton *btnNetwork = new QPushButton(u8"网络调试页->", serialPage);
    btnNetwork->setFixedSize(120, 35);
    connect(btnNetwork, &QPushButton::clicked, this, &Widget::switchToNetwork);

    //跳转到Bootloader页的按钮
    QPushButton *btnBootloader = new QPushButton(u8"BootloaderPage->",serialPage);
    btnBootloader->setFixedSize(140,35);
    connect(btnBootloader,&QPushButton::clicked,this,&Widget::switchToBootloader);


    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addWidget(title);
    topLayout->addStretch();
    topLayout->addWidget(btnNetwork);
    topLayout->addWidget(btnBootloader);

    // ********** 接收区 ******************************************************************
    receivedArea = new QPlainTextEdit(serialPage);
    //receivedArea->setFixedSize(30,20,800,400);
    receivedArea->setReadOnly(true);
    receivedArea->setPlaceholderText(u8"接收数据显示区...");

    QPushButton *clearReceiveArea = new QPushButton(u8"清空接收区", serialPage);
    //clearReceiveArea->setGeometry(680, 430,150, 50);
    connect(clearReceiveArea, &QPushButton::clicked, [=]() {
        receivedArea->clear();
    });
    // ********** 发送区 ******************************************************************
    sendArea = new QPlainTextEdit(serialPage);
    sendArea->setPlaceholderText(u8"输入要发送的数据...");
    sendArea->setMaximumHeight(100);

    sendButton = new QPushButton(u8"发送", serialPage);
    sendButton->setDisabled(true);
    sendButton->setFixedWidth(100);
    connect(sendButton, &QPushButton::clicked, [=]() {
        QString data = sendArea->toPlainText();
        if (data.isEmpty() && !addNewLine->isChecked())
            return; // 只有在既没数据也不发新行时才返回

        QByteArray finalBytes; // 定义一个唯一的缓冲区
        /*根据发送模式选择发送不同格式的数据*/
        if (sendMode->currentText() == "HEX") {

            for (int i = 0; i < data.size(); ++i) {
                if (data[i] == ' ')
                    continue;                                 // 跳过空格
                int num = data.mid(i, 2).toUInt(nullptr, 16); // 取两个字符转为16进制数
                i++;                                          // 跳过已处理的第二个字符
                finalBytes.append((char)num);
            }
            // 如果选中了发送新行, HEX 模式也要发送新行，通常是追加 0D 0A
            if (addNewLine->isChecked()) {
                finalBytes.append('\n');
            }
        } else {
            finalBytes = data.toLocal8Bit();
            // 如果选中了发送新行
            if (addNewLine->isChecked()) {
                if (!finalBytes.endsWith(' ')) { // 强行补丁:避免help <>打印命令语法时出现中间空行现象。
                    finalBytes.append(' ');
                }
                finalBytes.append("\n"); // 追加回车换行
            }
        }
        serialPort->write(finalBytes);
    });

    QPushButton *clearSendArea = new QPushButton(u8"清空发送区", serialPage);
    clearSendArea->setFixedWidth(100);
    connect(clearSendArea, &QPushButton::clicked, [=]() {
        sendArea->clear();
    });

    sendBinBtn = new QPushButton(u8"发送.bin文件", serialPage);
    sendBinBtn->setFixedWidth(120);
    connect(sendBinBtn, &QPushButton::clicked, this, &Widget::on_btnSelectAndSendBin_clicked);
    sendBinBtn->setDisabled(true);

    addNewLine = new QCheckBox(u8"发送新行", serialPage);
    addNewLine->setChecked(true); // 默认勾选

    QHBoxLayout *sendBtnLayout = new QHBoxLayout();
    sendBtnLayout->addWidget(addNewLine);
    sendBtnLayout->addWidget(clearSendArea);
    sendBtnLayout->addStretch();
    sendBtnLayout->addWidget(sendBinBtn);
    sendBtnLayout->addWidget(sendButton);

    // ********** 右侧配置区 ******************************************************************
    QGroupBox *configGroup = new QGroupBox(u8"串口配置",serialPage);
    QVBoxLayout *configLayout = new QVBoxLayout();

    // 串口号
    QHBoxLayout *portNumLayout = new QHBoxLayout();
    portNumLayout->addWidget(new QLabel(u8"串口号:", serialPage));
    portNumber = new QComboBox(serialPage);
    portNumLayout->addWidget(portNumber);
    configLayout->addLayout(portNumLayout);

    // 波特率
    QHBoxLayout *baudRateLayout = new QHBoxLayout();
    baudRateLayout->addWidget(new QLabel(u8"波特率:", serialPage));
    baudRate = new QComboBox(serialPage);
    baudRate->addItems({u8"115200", u8"9600",u8"4800"});
    baudRateLayout->addWidget(baudRate);
    configLayout->addLayout(baudRateLayout);

    // 数据位
    QHBoxLayout *dataSizeLayout = new QHBoxLayout();
    dataSizeLayout->addWidget(new QLabel(u8"数据位:", serialPage));
    dataSize = new QComboBox(serialPage);
    dataSize->addItem(u8"8");
    dataSizeLayout->addWidget(dataSize);
    configLayout->addLayout(dataSizeLayout);

    // 停止位
    QHBoxLayout *stopSizeLayout = new QHBoxLayout();
    stopSizeLayout->addWidget(new QLabel(u8"停止位:", serialPage));
    stopSize = new QComboBox(serialPage);
    stopSize->addItems({"1","1.5","2"});
    stopSizeLayout->addWidget(stopSize);
    configLayout->addLayout(stopSizeLayout);

    // 校验位
    QHBoxLayout *checkLayout = new QHBoxLayout();
    checkLayout->addWidget(new QLabel(u8"校验位:", serialPage));
    check = new QComboBox(serialPage);
    check->addItems({u8"无校验",u8"奇校验",u8"偶校验"});
    checkLayout->addWidget(check);
    configLayout->addLayout(checkLayout);

    // 接收格式
    QHBoxLayout *recvLayout = new QHBoxLayout();
    recvLayout->addWidget(new QLabel(u8"接收格式:", serialPage));
    receiveMode = new QComboBox(serialPage);
    receiveMode->addItems({"HEX", u8"文本"});
    receiveMode->setCurrentText(u8"文本");
    recvLayout->addWidget(receiveMode);
    configLayout->addLayout(recvLayout);

    // 发送格式
    QHBoxLayout *sendLayout = new QHBoxLayout();
    sendLayout->addWidget(new QLabel(u8"发送格式:", serialPage));
    sendMode = new QComboBox(serialPage);
    sendMode->addItems({"HEX", u8"文本"});
    sendMode->setCurrentText(u8"文本");
    sendLayout->addWidget(sendMode);
    configLayout->addLayout(sendLayout);

    // 连接按钮
    QHBoxLayout *connectLayout = new QHBoxLayout();
    Estbconnect = new QPushButton(u8"连接串口",serialPage);
    Estbconnect->setFixedHeight(45);
    Estbconnect->setStyleSheet("font-size:14px;font-weight:bold");
    connect(Estbconnect, &QPushButton::clicked, [=]() {
        if (portNumber->currentText() != " ") {
            sendButton->setDisabled(false);
            Estbconnect->setDisabled(true);
            Disconnect->setDisabled(false);
            sendBinBtn->setDisabled(false);
            /*连接*/
            USART();
        } else {
            QMessageBox::critical(this, QString::fromLocal8Bit("串口打开失败"),
                                  QString::fromLocal8Bit("请确认串口是否连接正确"));
        }
    });
    Disconnect = new QPushButton(u8"断开串口",serialPage);
    Disconnect->setFixedHeight(45);
    Disconnect->setStyleSheet("font-size:14px;font-weight:bold");
    connect(Disconnect, &QPushButton::clicked, [=]() {
        sendButton->setDisabled(true);   // 断开连接,失能发送按钮
        Estbconnect->setDisabled(false); // 使能串口连接按钮
        Disconnect->setDisabled(true);   // 失能串口断开按钮
        sendBinBtn->setDisabled(true);   // 断开连接,失能发送bin文件按钮
        /*断开连接*/
        serialPort->close();

        //版3--发送串口断开信号
        emit serialConnected(false);
        //版3--更新Bootloader页面状态
        if(bootloaderPage){
            bootloaderPage->updateSerialStatus(false);
        }
    });

    connectLayout->addWidget(Estbconnect);
    connectLayout->addWidget(Disconnect);
    configLayout->addLayout(connectLayout);

    configLayout->addStretch();
    configGroup->setLayout(configLayout);
    configGroup->setFixedWidth(250);

    // ********** 主布局 ******************************************************************
    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->addWidget(receivedArea);
    leftLayout->addWidget(clearReceiveArea);
    leftLayout->addWidget(sendArea);
    leftLayout->addLayout(sendBtnLayout);

    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->addLayout(leftLayout, 1);
    mainLayout->addWidget(configGroup);

    QVBoxLayout *rootLayout = new QVBoxLayout(serialPage);
    rootLayout->addLayout(topLayout);
    rootLayout->addLayout(mainLayout);
}

void Widget::USART() {
    // 1. 先获取 GBK 编码器
    codec = QTextCodec::codecForName("GBK");
    // 2. 从编码器创建一个解码器（它能记录状态，处理分包导致的乱码）
    if (codec) {
        decoder = codec->makeDecoder();
    }

    // --- 定于枚举类型的变量Baud，Data，Stop，Check
    // --- 修改点：在声明时初始化默认值 ---
    QSerialPort::BaudRate Baud = QSerialPort::Baud115200;
    QSerialPort::DataBits Databit = QSerialPort::Data8;
    QSerialPort::StopBits Stop = QSerialPort::OneStop;
    QSerialPort::Parity Check = QSerialPort::NoParity;

    QString portnum = portNumber->currentText(); // 获取各个下拉选框当前的值
    QString baud = baudRate->currentText();
    QString data = dataSize->currentText();
    QString stop = stopSize->currentText();
    QString ch = check->currentText();

    if (baud == "115200")
        Baud = QSerialPort::Baud115200;
    else if (baud == "9600")
        Baud = QSerialPort::Baud9600;
    else if (baud == "4800")
        Baud = QSerialPort::Baud4800;

    if (data == "8")
        Databit = QSerialPort::Data8;

    if (stop == "1")
        Stop = QSerialPort::OneStop;
    else if (stop == "1.5")
        Stop = QSerialPort::OneAndHalfStop;
    else if (stop == "2")
        Stop = QSerialPort::TwoStop;

    if (ch == QString::fromLocal8Bit("无校验"))
        Check = QSerialPort::NoParity;
    else if (ch == QString::fromLocal8Bit("奇校验"))
        Check = QSerialPort::OddParity;
    else if (ch == QString::fromLocal8Bit("偶校验"))
        Check = QSerialPort::EvenParity;

    /*将选择的数据送入"SerialPort"*/
    serialPort = new QSerialPort(this);
    serialPort->setBaudRate(Baud);
    serialPort->setDataBits(Databit);
    serialPort->setParity(Check);
    serialPort->setStopBits(Stop);
    serialPort->setPortName(portnum);

    if (serialPort->open(QSerialPort::ReadWrite)) {

        connect(serialPort, &QSerialPort::readyRead, this, &Widget::HandleSerialData);

        //版3:发送串口连接成功信号
        emit serialConnected(true);
        emit serialConfigChanged(portnum,baud);

        //版3:更新Bootloader页面状态
        if(bootloaderPage){
            bootloaderPage->updateSerialStatus(true);
            bootloaderPage->updateSerialConfig(portnum,baud);
        }

        //版2:bootloaderPage->setSerialPort(serialPort);
    } else {
        QMessageBox::critical(this, QString::fromLocal8Bit("串口打开失败"),
                              QString::fromLocal8Bit("请确认串口是否连接正确"));
    }
}

void Widget::timerEvent(QTimerEvent *e) {
    Q_UNUSED(e); // 消除 e 未使用的警告
    QVector<QString> temp;
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
        temp.push_back(info.portName());
    }
    std::sort(temp.begin(), temp.end());
    if (temp != ports) {
        this->portNumber->clear();
        this->ports = temp;
        for (auto &a : ports)
            this->portNumber->addItem(a);
    }
}

void Widget::HandleSerialData() {
    QByteArray data = serialPort->readAll();
    if (data.isEmpty())
        return;

//        QString hexData = data.toHex(' ').toUpper();
//        receivedArea->appendPlainText(QString("收到%1字节:%2")
//                                     .arg(data.size())
//                                     .arg(hexData));

    // 如果 sz 进程在运行，数据由它处理
    if (szProcess && szProcess->state() == QProcess::Running) {
        szProcess->write(data); // 将串口数据喂给 sz
        return;
    }

    // 普通助手模式 ---
    // 1. 处理滚动条逻辑（判断是否在底部）
    QScrollBar *vScrollBar = receivedArea->verticalScrollBar();
    bool isAtBottom = (vScrollBar->value() >= vScrollBar->maximum() - 5);

    // 2. 字符解码与插入
    QString str;
    if (receiveMode->currentText() == "HEX") {
        str = data.toHex(' ').toUpper() + " ";
    } else {
        str = decoder ? decoder->toUnicode(data) : QString::fromLocal8Bit(data);
    }

    // 3. UI 显示
    receivedArea->moveCursor(QTextCursor::End);
    receivedArea->insertPlainText(str);

    // 4. 滚动条自动跟踪
    if (isAtBottom) {
        receivedArea->ensureCursorVisible();
        vScrollBar->setValue(vScrollBar->maximum());
    }

    //版3新增--发送信号给Boot1oader页面
    emit serialDataReceived(str);

}

void Widget::on_btnSelectAndSendBin_clicked() {
    if (!serialPort || !serialPort->isOpen()) {
        QMessageBox::warning(this, u8"提示", u8"请先连接串口");
        return;
    }

    if (szProcess && szProcess->state() == QProcess::Running) {
        QMessageBox::warning(this, u8"提示", u8"传输正在进行中");
        return;
    }

    QString filePath = QFileDialog::getOpenFileName(this, u8"选择要发送的.bin文件", "", "*.bin");
    if (filePath.isEmpty())
        return;

    pendingFilePath = filePath;

    receivedArea->appendPlainText(u8"\n>>> 准备发送: " + QFileInfo(filePath).fileName());

    // 等待用户在 STM32 端输入 rz(0.1s)
    QTimer::singleShot(100, this, &Widget::startSzTransfer);
}

void Widget::startSzTransfer() {
    if (pendingFilePath.isEmpty())
        return;

    receivedArea->appendPlainText(u8">>> 启动 Z-modem 传输...\n");

    szProcess = new QProcess(this);

    //sz 的输出重定向到串口
    connect(szProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        QByteArray data = szProcess->readAllStandardOutput();

        //调试信息,Qt向32发送的数据(传输.bin文件阶段)
//        QString hexData = data.toHex(' ').toUpper();
//        receivedArea->appendPlainText(QString("收到%1字节:%2")
//                                     .arg(data.size())
//                                     .arg(hexData));

        if (serialPort && serialPort->isOpen()) {
            serialPort->write(data);
        }
    });

    //sz 的错误输出显示到界面
    connect(szProcess, &QProcess::readyReadStandardError, this, [this]() {
        QString err = QString::fromLocal8Bit(szProcess->readAllStandardError());
        receivedArea->appendPlainText(u8"[sz错误] " + err);
    });

    //传输完成处理
    connect(szProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
                Q_UNUSED(exitStatus);

                if (exitCode == 0) {
                    receivedArea->appendPlainText(u8"\n--- 传输成功 ---\n");
                } else {
                    receivedArea->appendPlainText(u8"\n--- 传输失败(错误码: " +
                                                  QString::number(exitCode) + u8") ---\n");
                }

                pendingFilePath.clear();
                szProcess->deleteLater();
                szProcess = nullptr;
            });

    // 构建 sz 命令
    QString program = "sz"; // Windows 下用 "sz.exe"
    QStringList arguments;
    arguments << "-b"             // 二进制模式
              << "-v"             // 详细输出
              << pendingFilePath; // 文件路径

#ifdef Q_OS_WIN
    program = "sz.exe";
#endif

    szProcess->start(program, arguments);

    if (!szProcess->waitForStarted(3000)) {
        receivedArea->appendPlainText(u8"错误: 无法启动 sz 工具\n");
        receivedArea->appendPlainText(u8"请确保已安装 lrzsz 并在 PATH 中\n");
        szProcess->deleteLater();
        szProcess = nullptr;
    }
}

void Widget::initPages() {
    stackedWidget = new QStackedWidget(this);
    //stackedWidget->setGeometry(0,0,1200,750);
    stackedWidget->setFixedSize(1200, 750);

    // 创建串口页
    serialPage = new QWidget();
    serialPage->setFixedSize(1200,750);
    initSerialPage();

    // 创建网络页
    networkPage = new NetworkPage();
    connect(networkPage, &NetworkPage::backToMain, this, &Widget::switchToSerial);

    // 创建Bootloader页
    bootloaderPage = new BootloaderPage(this);
    connect(bootloaderPage,&BootloaderPage::backToMain,this,&Widget::switchToSerialFromBootloader);

    // 添加到堆栈
    stackedWidget->addWidget(serialPage);   // index 0
    stackedWidget->addWidget(networkPage);  // index 1
    stackedWidget->addWidget(bootloaderPage);//index 2

    stackedWidget->setCurrentIndex(0);  // 默认显示串口页

    this->startTimer(1000);//定时检测识别串口
}

void Widget::initSerialPage() {
    // 【重要】将原来 Widget 构造函数中的所有初始化代码移到这里
    // 注意：所有控件的 parent 要改为 serialPage

    //    ReceiveAreaInit();
    //    SendAreaInit();
    //    SetupInit();
    //    beginUART();
        SetupUiInit();
}

void Widget::switchToNetwork() {
    stackedWidget->setCurrentWidget(networkPage);
}

void Widget::switchToSerial() {
    stackedWidget->setCurrentWidget(serialPage);
}

void Widget::switchToBootloader(){
    //检查串口状态
    if(!serialPort || !serialPort->isOpen()){
        QMessageBox::warning(this,u8"提示",u8"请先连接串口再进入Bootloader页面");
        //return;
    }
    stackedWidget->setCurrentWidget(bootloaderPage);
}

void Widget::switchToSerialFromBootloader(){
    stackedWidget->setCurrentWidget(serialPage);
}

void Widget::triggerSendBin(){
    //调用现有的发送.bin文件功能
    on_btnSelectAndSendBin_clicked();
}


/* 第2版的跳转函数switchToBootloader
void Widget::switchToBootloader()
{
    if (!serialPort->isOpen()) {
        QMessageBox::warning(serialPage, "错误", "串口未连接，无法进入Bootloader模式！");
        return;
    }

    //共享串口和配置
    bootloaderPage->setSerialPort(serialPort);
    bootloaderPage->setSerialConfig(portNumber,baudRate,dataSize,stopSize,check);
    stackedWidget->setCurrentWidget(bootloaderPage);
}*/

Widget::Widget(QWidget *parent)
    : QWidget(parent) {
    this->setFixedSize(1200, 750); // 设置窗口大小，宽1200，高750
    this->setWindowTitle(u8"Bootloader调试");

    initPages();//初始化界面

}

Widget::~Widget() {
    // 确保退出时关闭串口
    if (serialPort && serialPort->isOpen()) {
        serialPort->close();
    }
}
