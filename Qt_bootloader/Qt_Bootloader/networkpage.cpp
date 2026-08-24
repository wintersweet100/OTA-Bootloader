#include "networkpage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QMessageBox>

NetworkPage::NetworkPage(QWidget *parent)
  : QWidget(parent)
  , tcpSocket(nullptr)
  , tcpServer(nullptr)
  , udpSocket(nullptr)
  , szProcess(nullptr)
  , decoder(nullptr)
  , isConnected(false)
{
    QTextCodec *codec = QTextCodec::codecForName("GBK");
    if (codec) {
        decoder = codec->makeDecoder();
    }

    initUI();
}

NetworkPage::~NetworkPage() {
    cleanup();
}

//UI界面
void NetworkPage::initUI() {
    // ========== 顶部标题和返回按钮 ==========
    QHBoxLayout *topLayout = new QHBoxLayout();

    QLabel *title = new QLabel(u8"网络调试助手", this);
    title->setStyleSheet("font-size: 18px; font-weight: bold;");

    btnBack = new QPushButton(u8"← 返回串口页", this);
    btnBack->setFixedSize(120, 35);
    connect(btnBack, &QPushButton::clicked, this, &NetworkPage::backToMain);

    topLayout->addWidget(title);
    topLayout->addStretch();
    topLayout->addWidget(btnBack);

    // ========== 接收区 ==========
    receiveArea = new QPlainTextEdit(this);
    receiveArea->setReadOnly(true);
    receiveArea->setPlaceholderText(u8"接收数据显示区...");

    btnClearRecv = new QPushButton(u8"清空接收区", this);
    connect(btnClearRecv, &QPushButton::clicked, receiveArea, &QPlainTextEdit::clear);

    // ========== 发送区 ==========
    sendArea = new QPlainTextEdit(this);
    sendArea->setPlaceholderText(u8"输入要发送的数据...");
    sendArea->setMaximumHeight(100);

    btnClearSend = new QPushButton(u8"清空发送区", this);
    btnClearSend->setFixedWidth(100);
    connect(btnClearSend, &QPushButton::clicked, sendArea, &QPlainTextEdit::clear);

    btnSend = new QPushButton(u8"发送", this);
    btnSend->setEnabled(false);//默认失能
    btnSend->setFixedWidth(100);
    connect(btnSend, &QPushButton::clicked, this, &NetworkPage::onSendClicked);

    btnSendBin = new QPushButton(u8"发送.bin文件",this);
    btnSendBin->setEnabled(false);
    btnSendBin->setFixedWidth(120);
    connect(btnSendBin,&QPushButton::clicked,this,&NetworkPage::onSendBinClicked);

    addNexLine = new QCheckBox(u8"发送新行",this);
    addNexLine->setChecked(true);

    QHBoxLayout *sendBtnLayout = new QHBoxLayout();
    sendBtnLayout->addWidget(addNexLine);
    sendBtnLayout->addWidget(btnClearSend);
    sendBtnLayout->addStretch();
    sendBtnLayout->addWidget(btnSendBin);
    sendBtnLayout->addWidget(btnSend);

    // ========== 右侧配置区 ==========
    QGroupBox *configGroup = new QGroupBox(u8"网络配置", this);
    QVBoxLayout *configLayout = new QVBoxLayout();

    // 协议类型
    QHBoxLayout *protocolLayout = new QHBoxLayout();
    protocolLayout->addWidget(new QLabel(u8"协议:", this));
    protocolType = new QComboBox(this);
    protocolType->addItems({"TCP", "UDP"});
    connect(protocolType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &NetworkPage::onProtocolChanged);
    protocolLayout->addWidget(protocolType);
    configLayout->addLayout(protocolLayout);

    // 客户端/服务器
    QHBoxLayout *clientLayout = new QHBoxLayout();
    clientLayout->addWidget(new QLabel(u8"模式:", this));
    clientType = new QComboBox(this);
    clientType->addItems({u8"客户端", u8"服务器"});
    clientLayout->addWidget(clientType);
    configLayout->addLayout(clientLayout);

    // IP地址
    QHBoxLayout *ipLayout = new QHBoxLayout();
    ipLayout->addWidget(new QLabel(u8"IP地址:", this));
    ipAddress = new QLineEdit("192.168.127.23",this);
    ipLayout->addWidget(ipAddress);
    configLayout->addLayout(ipLayout);

    // 端口
    QHBoxLayout *portLayout = new QHBoxLayout();
    portLayout->addWidget(new QLabel(u8"端口:", this));
    port = new QLineEdit("8000", this);
    port->setValidator(new QIntValidator(1, 65535, this));
    portLayout->addWidget(port);
    configLayout->addLayout(portLayout);

    // 接收格式
    QHBoxLayout *recvLayout = new QHBoxLayout();
    recvLayout->addWidget(new QLabel(u8"接收格式:", this));
    receiveMode = new QComboBox(this);
    receiveMode->addItems({"HEX", u8"文本"});
    receiveMode->setCurrentText(u8"文本");
    recvLayout->addWidget(receiveMode);
    configLayout->addLayout(recvLayout);

    // 发送格式
    QHBoxLayout *sendLayout = new QHBoxLayout();
    sendLayout->addWidget(new QLabel(u8"发送格式:", this));
    sendMode = new QComboBox(this);
    sendMode->addItems({"HEX", u8"文本"});
    sendMode->setCurrentText(u8"文本");
    sendLayout->addWidget(sendMode);
    configLayout->addLayout(sendLayout);

    // 连接按钮
    btnConnect = new QPushButton(u8"连接", this);
    btnConnect->setFixedHeight(45);
    btnConnect->setStyleSheet("font-size: 14px; font-weight: bold;");
    connect(btnConnect, &QPushButton::clicked, this, &NetworkPage::onConnectClicked);
    configLayout->addWidget(btnConnect);

    configLayout->addStretch();
    configGroup->setLayout(configLayout);
    configGroup->setFixedWidth(250);

    // ========== 主布局 ==========
    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->addWidget(receiveArea);
    leftLayout->addWidget(btnClearRecv);
    leftLayout->addWidget(sendArea);
    leftLayout->addLayout(sendBtnLayout);

    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->addLayout(leftLayout, 1);
    mainLayout->addWidget(configGroup);

    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->addLayout(topLayout);
    rootLayout->addLayout(mainLayout);
}

//TCP、UDP协议选择下拉框的槽函数
void NetworkPage::onProtocolChanged(int index) {
    // TCP服务器模式不需要IP
    if (index == 0 && clientType->currentText() == u8"服务器") {
        ipAddress->setEnabled(false);
    } else {
        ipAddress->setEnabled(true);
    }
}

//点击连接按钮时触发的槽函数
void NetworkPage::onConnectClicked() {
    if (isConnected) {
        // 断开连接
        cleanup();
        btnConnect->setText(u8"连接");
        btnSend->setEnabled(false);
        isConnected = false;
        receiveArea->appendPlainText(u8"\n--- 已断开连接 ---\n");
        return;
    }

    QString protocol = protocolType->currentText();
    QString mode = clientType->currentText();
    QString ip = ipAddress->text();
    quint16 portNum = port->text().toUShort();

    if (protocol == "TCP") {
        if (mode == u8"客户端") {
            // TCP客户端
            tcpSocket = new QTcpSocket(this);

            //建立连接时触发
            connect(tcpSocket, &QTcpSocket::connected, this, &NetworkPage::onTcpConnected);
            //接收到服务器发来的数据时触发
            connect(tcpSocket, &QTcpSocket::readyRead, this, &NetworkPage::onTcpReadyRead);

            tcpSocket->setSocketOption(QAbstractSocket::LowDelayOption, 1);//禁用Nagle算法，减少延迟
            tcpSocket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);//设置Keep-Alive 保持连接

            //断开连接时触发
            connect(tcpSocket, &QTcpSocket::disconnected, this, &NetworkPage::onTcpDisconnected);

            receiveArea->appendPlainText(u8"正在连接 " + ip + ":" + QString::number(portNum) + "...\n");
            tcpSocket->connectToHost(ip, portNum);

        } else {
            // TCP服务器
            tcpServer = new QTcpServer(this);
            if (tcpServer->listen(QHostAddress::Any, portNum)) {
                receiveArea->appendPlainText(u8"TCP服务器已启动，监听端口: " + QString::number(portNum) + "\n");
                btnConnect->setText(u8"断开");
                isConnected = true;

                connect(tcpServer, &QTcpServer::newConnection, this, [this]() {
                    tcpSocket = tcpServer->nextPendingConnection();

                    tcpSocket->setSocketOption(QAbstractSocket::LowDelayOption,1);//禁用Nagle算法，减少延迟
                    tcpSocket->setSocketOption(QAbstractSocket::KeepAliveOption,1);//设置Keep-Alive 保持连接

                    connect(tcpSocket, &QTcpSocket::disconnected, this, &NetworkPage::onTcpDisconnected);
                    connect(tcpSocket, &QTcpSocket::readyRead, this, &NetworkPage::onTcpReadyRead);

                    receiveArea->appendPlainText(u8"客户端已连接: " +
                        tcpSocket->peerAddress().toString() + ":" +
                        QString::number(tcpSocket->peerPort()) + "\n");
                    btnSend->setEnabled(true);
                    btnSendBin->setEnabled(true);
                });
            } else {
                QMessageBox::critical(this, u8"错误", u8"无法启动服务器: " + tcpServer->errorString());
            }
        }
    } else {
        // UDP
        udpSocket = new QUdpSocket(this);
        if (udpSocket->bind(QHostAddress::Any, portNum)) {
            connect(udpSocket, &QUdpSocket::readyRead, this, &NetworkPage::onUdpReadyRead);
            receiveArea->appendPlainText(u8"UDP已绑定端口: " + QString::number(portNum) + "\n");
            btnConnect->setText(u8"断开");
            btnSend->setEnabled(true);
            isConnected = true;
        } else {
            QMessageBox::critical(this, u8"错误", u8"UDP绑定失败");
        }
    }
}

//TCP建立连接时触发的槽函数
void NetworkPage::onTcpConnected() {
    receiveArea->appendPlainText(u8"TCP连接成功\n");
    btnConnect->setText(u8"断开");
    btnSend->setEnabled(true);
    btnSendBin->setEnabled(true);
    isConnected = true;
}

//TCP断开连接时触发的槽函数
void NetworkPage::onTcpDisconnected() {
    receiveArea->appendPlainText(u8"\nTCP连接已断开\n");
    btnConnect->setText(u8"连接");
    btnSend->setEnabled(false);
    btnSendBin->setEnabled(false);
    isConnected = false;
}

void NetworkPage::onTcpReadyRead() {
    if (!tcpSocket) return;

    QByteArray data = tcpSocket->readAll();

//    qDebug()<<"[WiFi]收到数据，长度:"<<data.size();
//    qDebug()<<"[WiFi]数据内容:"<<data.toHex();

    //如果sz进程在运行，将数据喂给它
    if(szProcess && szProcess->state() == QProcess::Running)
    {
        szProcess->write(data);
        return;
    }

    QScrollBar *vScrollBar = receiveArea->verticalScrollBar();
    bool isAtBottom = (vScrollBar->value() >= vScrollBar->maximum() - 5);

    QString str;
    if (receiveMode->currentText() == "HEX") {
        str = data.toHex(' ').toUpper() + " ";
    } else {
        str = decoder ? decoder->toUnicode(data) : QString::fromLocal8Bit(data);
    }

    receiveArea->moveCursor(QTextCursor::End);
    receiveArea->insertPlainText(str);

    if (isAtBottom) {
        receiveArea->ensureCursorVisible();
        vScrollBar->setValue(vScrollBar->maximum());
    }
}

void NetworkPage::onUdpReadyRead() {
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray data;
        data.resize(udpSocket->pendingDatagramSize());

        QHostAddress sender;
        quint16 senderPort;
        udpSocket->readDatagram(data.data(), data.size(), &sender, &senderPort);

        QString header = u8"[来自 " + sender.toString() + ":" + QString::number(senderPort) + "] ";

        QString str;
        if (receiveMode->currentText() == "HEX") {
            str = header + data.toHex(' ').toUpper() + "\n";
        } else {
            str = header + (decoder ? decoder->toUnicode(data) : QString::fromLocal8Bit(data)) + "\n";
        }

        receiveArea->appendPlainText(str);
    }
}

void NetworkPage::onSendClicked() {
    QString text = sendArea->toPlainText();
    if (text.isEmpty()&& !addNexLine->isChecked()) return;

    QByteArray data;

    if (sendMode->currentText() == "HEX") {
        for (int i = 0; i < text.size(); ++i) {
            if (text[i] == ' ') continue;
            int num = text.mid(i, 2).toUInt(nullptr, 16);
            i++;
            data.append((char)num);
        }
        if(addNexLine->isChecked()){
            data.append('\n');
        }

    } else {
        data = text.toLocal8Bit();
        if(addNexLine->isChecked()){
            data.append('\n');
        }
    }

    if (protocolType->currentText() == "TCP") {
        if (tcpSocket && tcpSocket->state() == QTcpSocket::ConnectedState) {
            tcpSocket->write(data);
            tcpSocket->flush();
        }
    } else {
        // UDP
        QString ip = ipAddress->text();
        quint16 portNum = port->text().toUShort();
        udpSocket->writeDatagram(data, QHostAddress(ip), portNum);
    }
}

void NetworkPage::onSendBinClicked()
{
    if(protocolType->currentText()!= "TCP"){
        QMessageBox::warning(this,u8"提示",u8"Z-modem仅支持TCP连接");
        return;
    }

    if (!tcpSocket || tcpSocket->state() != QTcpSocket::ConnectedState) {
        QMessageBox::warning(this, u8"提示", u8"请先建立 TCP 连接");
        return;
    }

    if (szProcess && szProcess->state() == QProcess::Running) {
        QMessageBox::warning(this, u8"提示", u8"传输正在进行中");
        return;
    }

    // 选择文件
    QString filePath = QFileDialog::getOpenFileName(this, u8"选择.bin文件", "", "*.bin");
    if (filePath.isEmpty())
        return;

    pendingFilePath = filePath;

    receiveArea->appendPlainText(u8"\n>>> 准备发送: " + QFileInfo(filePath).fileName());
    //receiveArea->appendPlainText(u8"请在目标设备执行 'rz' 命令...\n");

    // 等待 0.1 秒后启动（给用户时间在 STM32 端输入 rz）
    QTimer::singleShot(100, this, &NetworkPage::startZmodemTransfer);
}

void NetworkPage::cleanup() {
    //清理sz进程
    if(szProcess){
        if(szProcess->state() == QProcess::Running){
            szProcess->terminate();
            szProcess->waitForFinished(1000);
        }
        szProcess->deleteLater();
        szProcess = nullptr;
    }

    if (tcpSocket) {
        tcpSocket->disconnectFromHost();
        tcpSocket->deleteLater();
        tcpSocket = nullptr;
    }
    if (tcpServer) {
        tcpServer->close();
        tcpServer->deleteLater();
        tcpServer = nullptr;
    }
    if (udpSocket) {
        udpSocket->close();
        udpSocket->deleteLater();
        udpSocket = nullptr;
    }
}

void NetworkPage::startZmodemTransfer()
{
    if (pendingFilePath.isEmpty())
        return;

    if (!tcpSocket || tcpSocket->state() != QTcpSocket::ConnectedState) {
        QMessageBox::warning(this, u8"错误", u8"TCP 连接已断开");
        pendingFilePath.clear();
        return;
    }

    receiveArea->appendPlainText(u8">>> 启动 Z-modem 传输...\n");

    szProcess = new QProcess(this);

    // 【关键1】sz 的输出重定向到 TCP Socket
    connect(szProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        QByteArray data = szProcess->readAllStandardOutput();
        if (tcpSocket && tcpSocket->state() == QTcpSocket::ConnectedState) {
            tcpSocket->write(data);
            tcpSocket->flush();//使数据立即发送，不对数据进行缓冲
        }
    });

    // 【关键2】sz 的错误输出显示到界面
    connect(szProcess, &QProcess::readyReadStandardError, this, [this]() {
        QString err = QString::fromLocal8Bit(szProcess->readAllStandardError());
        receiveArea->appendPlainText(u8"[sz错误] " + err);
    });

    // 【关键3】 传输完成处理
    connect(szProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        Q_UNUSED(exitStatus);

        if (exitCode == 0) {
            receiveArea->appendPlainText(u8"\n--- 传输成功 ---\n");
        } else {
            receiveArea->appendPlainText(u8"\n--- 传输失败(错误码: " +
                                        QString::number(exitCode) + u8") ---\n");
        }

        pendingFilePath.clear();
        szProcess->deleteLater();
        szProcess = nullptr;
    });

    // 构建 sz 命令
    QString program;
#ifdef Q_OS_WIN
    program = QCoreApplication::applicationDirPath() + "/sz.exe";
    if (!QFile::exists(program)) {
        receiveArea->appendPlainText(u8"错误: 找不到 sz.exe\n");
        receiveArea->appendPlainText(u8"路径: " + program + "\n");
        szProcess->deleteLater();
        szProcess = nullptr;
        return;
    }
#else
    program = "sz";
#endif

    QStringList arguments;
    arguments << "-b"               // 二进制模式
              << "-v"               // 详细输出
              << pendingFilePath;

    qDebug() << "Starting sz:" << program << arguments;

    szProcess->start(program, arguments);

    if (!szProcess->waitForStarted(3000)) {
        receiveArea->appendPlainText(u8"错误: 无法启动 sz 工具\n");
        szProcess->deleteLater();
        szProcess = nullptr;
    }
}

