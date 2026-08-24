#include "bootloaderpage.h"
#include "widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QTextCodec>
#include <QScrollBar>
#include <QSerialPort>
#include <QCoreApplication>


BootloaderPage::BootloaderPage(Widget *parentWidget, QWidget *parent)
    : QWidget(parent)
    , isWiFiConnected(false)
    , isEthernetConnected(false)
    , wifiSocket(nullptr)
    , ethernetSocket(nullptr)
    , szProcess(nullptr)
    , decoder(nullptr)
    , parentWidget(parentWidget)
{
    // 初始化GBK解码器
    QTextCodec *codec = QTextCodec::codecForName("GBK");
    if (codec) {
        decoder = codec->makeDecoder();
    }

    initUI();
    if (parentWidget && !useSerial->isChecked()) {
        connect(parentWidget, &Widget::serialDataReceived,
                this, &BootloaderPage::appendSerialData);
    }//将串口助手界面的接收区数据与Bootloader命令界面的接收区数据同步起来(前者发送给后者)
}

BootloaderPage::~BootloaderPage()
{
    cleanup();
}

void BootloaderPage::initUI()
{
    // ========== 顶部标题和返回按钮 ==========
    QHBoxLayout *topLayout = new QHBoxLayout();

    QLabel *title = new QLabel(u8"Bootloader控制台", this);
    title->setStyleSheet("font-size: 18px; font-weight: bold;");

    btnBack = new QPushButton(u8"← 返回主页", this);
    btnBack->setFixedSize(120, 35);
    connect(btnBack, &QPushButton::clicked, this, &BootloaderPage::onBackClicked);

    topLayout->addWidget(title);
    topLayout->addStretch();
    topLayout->addWidget(btnBack);

    // ========== 通信配置区（三列布局）==========
    QHBoxLayout *configLayout = new QHBoxLayout();

    // --- 串口配置 ---
    QGroupBox *serialGroup = new QGroupBox(u8"串口通信", this);
    QVBoxLayout *serialLayout = new QVBoxLayout();

    lblSerialPort = new QLabel(u8"端口: 未连接", this);
    lblBaudRate = new QLabel(u8"波特率: --", this);
    useSerial = new QCheckBox(u8"启用串口通信", this);
    useSerial->setChecked(false);
    useSerial->setEnabled(false); // 初始禁用，等待Widget连接
    connect(useSerial, &QCheckBox::stateChanged, this, &BootloaderPage::onSerialCheckChanged);

    serialLayout->addWidget(lblSerialPort);
    serialLayout->addWidget(lblBaudRate);
    serialLayout->addWidget(useSerial);
    serialLayout->addStretch();
    serialGroup->setLayout(serialLayout);
    serialGroup->setFixedWidth(250);

    // --- WiFi配置 ---
    QGroupBox *wifiGroup = new QGroupBox(u8"WiFi通信", this);
    QVBoxLayout *wifiLayout = new QVBoxLayout();

    QHBoxLayout *wifiIPLayout = new QHBoxLayout();
    wifiIPLayout->addWidget(new QLabel(u8"IP:", this));
    wifiIP = new QLineEdit("192.168.127.23", this);
    wifiIPLayout->addWidget(wifiIP);

    QHBoxLayout *wifiPortLayout = new QHBoxLayout();
    wifiPortLayout->addWidget(new QLabel(u8"端口:", this));
    wifiPort = new QLineEdit("8000", this);
    wifiPortLayout->addWidget(wifiPort);

    useWiFi = new QCheckBox(u8"启用WiFi通信", this);
    connect(useWiFi, &QCheckBox::stateChanged, this, &BootloaderPage::onWiFiCheckChanged);

    btnWiFiConnect = new QPushButton(u8"连接", this);
    btnWiFiConnect->setFixedHeight(35);
    connect(btnWiFiConnect, &QPushButton::clicked, this, &BootloaderPage::onWiFiConnectClicked);

    wifiLayout->addLayout(wifiIPLayout);
    wifiLayout->addLayout(wifiPortLayout);
    wifiLayout->addWidget(useWiFi);
    wifiLayout->addWidget(btnWiFiConnect);
    wifiLayout->addStretch();
    wifiGroup->setLayout(wifiLayout);
    wifiGroup->setFixedWidth(250);

    // --- 以太网配置 ---
    QGroupBox *ethernetGroup = new QGroupBox(u8"以太网通信", this);
    QVBoxLayout *ethernetLayout = new QVBoxLayout();

    QHBoxLayout *ethIPLayout = new QHBoxLayout();
    ethIPLayout->addWidget(new QLabel(u8"IP:", this));
    ethernetIP = new QLineEdit("192.168.1.88", this);
    ethIPLayout->addWidget(ethernetIP);

    QHBoxLayout *ethPortLayout = new QHBoxLayout();
    ethPortLayout->addWidget(new QLabel(u8"端口:", this));
    ethernetPort = new QLineEdit("8000", this);
    ethPortLayout->addWidget(ethernetPort);

    useEthernet = new QCheckBox(u8"启用以太网通信", this);
    connect(useEthernet, &QCheckBox::stateChanged, this, &BootloaderPage::onEthernetCheckChanged);

    btnEthernetConnect = new QPushButton(u8"连接", this);
    btnEthernetConnect->setFixedHeight(35);
    connect(btnEthernetConnect, &QPushButton::clicked, this, &BootloaderPage::onEthernetConnectClicked);

    ethernetLayout->addLayout(ethIPLayout);
    ethernetLayout->addLayout(ethPortLayout);
    ethernetLayout->addWidget(useEthernet);
    ethernetLayout->addWidget(btnEthernetConnect);
    ethernetLayout->addStretch();
    ethernetGroup->setLayout(ethernetLayout);
    ethernetGroup->setFixedWidth(250);

    configLayout->addWidget(serialGroup);
    configLayout->addWidget(wifiGroup);
    configLayout->addWidget(ethernetGroup);
    configLayout->addStretch();

    // ========== 地址信息区 ==========
    QGroupBox *addrGroup = new QGroupBox(u8"Flash地址信息", this);
    QHBoxLayout *addrLayout = new QHBoxLayout();

    lblRunAddr = new QLabel(u8"运行区: 0x08009000", this);
    lblRunAddr->setStyleSheet("font-weight: bold; color: #2E86C1;");
    lblAAddr = new QLabel(u8"A存储区: 0x08029000", this);
    lblAAddr->setStyleSheet("font-weight: bold; color: #27AE60;");
    lblBAddr = new QLabel(u8"B存储区: 0x08050000", this);
    lblBAddr->setStyleSheet("font-weight: bold; color: #E67E22;");

    addrLayout->addWidget(lblRunAddr);
    addrLayout->addSpacing(20);
    addrLayout->addWidget(lblAAddr);
    addrLayout->addSpacing(20);
    addrLayout->addWidget(lblBAddr);
    addrLayout->addStretch();
    addrGroup->setLayout(addrLayout);

    // ========== 功能操作区 ==========
    QGroupBox *funcGroup = new QGroupBox(u8"功能操作", this);
    QGridLayout *funcLayout = new QGridLayout();

    btnEraseRun = new QPushButton(u8"擦除运行区", this);
    btnEraseRun->setFixedHeight(40);
    btnEraseRun->setEnabled(false);
    connect(btnEraseRun, &QPushButton::clicked, this, &BootloaderPage::onEraseRunClicked);

    btnWriteAToRun = new QPushButton(u8"A区→运行区", this);
    btnWriteAToRun->setFixedHeight(40);
    btnWriteAToRun->setEnabled(false);
    connect(btnWriteAToRun, &QPushButton::clicked, this, &BootloaderPage::onWriteAToRunClicked);

    btnWriteBToRun = new QPushButton(u8"B区→运行区", this);
    btnWriteBToRun->setFixedHeight(40);
    btnWriteBToRun->setEnabled(false);
    connect(btnWriteBToRun, &QPushButton::clicked, this, &BootloaderPage::onWriteBToRunClicked);

    btnSendBin = new QPushButton(u8"发送.bin文件", this);
    btnSendBin->setFixedHeight(40);
    btnSendBin->setEnabled(false);
    connect(btnSendBin, &QPushButton::clicked, this, &BootloaderPage::onSendBinClicked);

    btnflashwriteToRun = new QPushButton(u8"接收的bin文件写入运行区", this);
    btnflashwriteToRun->setFixedHeight(40);
    btnflashwriteToRun->setEnabled(false);
    connect(btnflashwriteToRun, &QPushButton::clicked, this, &BootloaderPage::FlashwriteToRun);

    btnflashwriteToA = new QPushButton(u8"接收的bin文件写入A存储区", this);
    btnflashwriteToA->setFixedHeight(40);
    btnflashwriteToA->setEnabled(false);
    connect(btnflashwriteToA, &QPushButton::clicked, this, &BootloaderPage::FlashwriteToAppA);

    btnflashwriteToB = new QPushButton(u8"接收的bin文件写入B存储区", this);
    btnflashwriteToB->setFixedHeight(40);
    btnflashwriteToB->setEnabled(false);
    connect(btnflashwriteToB, &QPushButton::clicked, this, &BootloaderPage::FlashwriteToAppB);

    btnsendHelp = new QPushButton(u8"Help", this);
    btnsendHelp->setFixedHeight(40);
    btnsendHelp->setEnabled(false);
    btnsendHelp->setStyleSheet("background-color: #FFDE80; color: white; font-weight: bold;");
    connect(btnsendHelp, &QPushButton::clicked, this, &BootloaderPage::onSendHelpClicked);

    btnEnterShell = new QPushButton(u8"进入Shell", this);
    btnEnterShell->setFixedHeight(40);
    btnEnterShell->setEnabled(false);
    btnEnterShell->setStyleSheet("background-color: #3498DB; color: white; font-weight: bold;");
    connect(btnEnterShell, &QPushButton::clicked, this, &BootloaderPage::onEnterShellClicked);

    btnGoRun = new QPushButton(u8"跳转到运行区", this);
    btnGoRun->setFixedHeight(40);
    btnGoRun->setEnabled(false);
    btnGoRun->setStyleSheet("background-color: #27AE60; color: white; font-weight: bold;");
    connect(btnGoRun, &QPushButton::clicked, this, &BootloaderPage::onGoRunClicked);

    funcLayout->addWidget(btnEraseRun, 0, 0);
    funcLayout->addWidget(btnWriteAToRun, 0, 1);
    funcLayout->addWidget(btnWriteBToRun, 0, 2);

    funcLayout->addWidget(btnSendBin, 1, 0);
    funcLayout->addWidget(btnflashwriteToRun,1,1);
    funcLayout->addWidget(btnflashwriteToA,1,2);
    funcLayout->addWidget(btnflashwriteToB,1,3);

    funcLayout->addWidget(btnsendHelp, 2, 0);
    funcLayout->addWidget(btnEnterShell, 2, 1);
    funcLayout->addWidget(btnGoRun, 2, 2);
    funcGroup->setLayout(funcLayout);

    // ========== 接收区 ==========
    receiveArea = new QPlainTextEdit(this);
    receiveArea->setReadOnly(true);
    receiveArea->setPlaceholderText(u8"数据接收区（支持串口/WiFi/以太网多路显示）...");

    btnClearRecv = new QPushButton(u8"清空接收区", this);
    connect(btnClearRecv, &QPushButton::clicked, receiveArea, &QPlainTextEdit::clear);

    // ========== 主布局 ==========
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(configLayout);
    mainLayout->addWidget(addrGroup);
    mainLayout->addWidget(funcGroup);
    mainLayout->addWidget(receiveArea, 1); // 接收区占据剩余空间
    mainLayout->addWidget(btnClearRecv);
}

void BootloaderPage::onBackClicked()
{
    emit backToMain();
}

void BootloaderPage::onSerialCheckChanged(int state)
{
    if (state == Qt::Checked) {
        // 启用串口通信时，发送初始化命令（如果需要）
        // 这里不需要发送，因为串口已经在Widget中初始化
        useSerial->setChecked(true);
        // 启用功能按钮
        btnEraseRun->setEnabled(true);
        btnWriteAToRun->setEnabled(true);
        btnWriteBToRun->setEnabled(true);
        btnSendBin->setEnabled(true);
        btnGoRun->setEnabled(true);
        btnEnterShell->setEnabled(true);
        btnsendHelp->setEnabled(true);
        btnflashwriteToRun->setEnabled(true);
        btnflashwriteToA->setEnabled(true);
        btnflashwriteToB->setEnabled(true);
    }
    else if(state == Qt::Unchecked && !useEthernet->isChecked() && !useWiFi->isChecked())
    {
        useSerial->setChecked(false);
        btnEraseRun->setEnabled(false);
        btnWriteAToRun->setEnabled(false);
        btnWriteBToRun->setEnabled(false);
        btnSendBin->setEnabled(false);
        btnGoRun->setEnabled(false);
        btnEnterShell->setEnabled(false);
        btnsendHelp->setEnabled(false);
        btnflashwriteToRun->setEnabled(false);
        btnflashwriteToA->setEnabled(false);
        btnflashwriteToB->setEnabled(false);
    }
}

//使能Wifi连接,通过串口向单片机发送数据，使其执行ESP-01S相关初始化
void BootloaderPage::onWiFiCheckChanged(int state)
{
    if (state == Qt::Checked) {
        // 通过串口发送WiFi初始化命令到STM32
        QString cmd = "InitWifi\n";

        if (parentWidget) {
            QSerialPort *serialPort = parentWidget->getSerialPort();
            if (serialPort && serialPort->isOpen()) {
                serialPort->write(cmd.toLocal8Bit());
                serialPort->flush();
                receiveArea->appendPlainText(u8"[串口] 已发送WiFi初始化命令: " + cmd.trimmed());
            } else {
                QMessageBox::warning(this, u8"提示", u8"请先连接串口");
                useWiFi->setChecked(false); // 取消勾选
            }
        }
    }
}

//使能以太网连接,通过串口向单片机发送数据，使其执行W5500相关初始化
void BootloaderPage::onEthernetCheckChanged(int state)
{
    if (state == Qt::Checked) {
        // 通过串口发送以太网初始化命令到STM32
        QString cmd = "InitEthnet\n";

        if (parentWidget) {
            QSerialPort *serialPort = parentWidget->getSerialPort();
            if (serialPort && serialPort->isOpen()) {
                serialPort->write(cmd.toLocal8Bit());
                serialPort->flush();
                receiveArea->appendPlainText(u8"[串口] 已发送以太网初始化命令: " + cmd.trimmed());
            } else {
                QMessageBox::warning(this, u8"提示", u8"请先连接串口");
                useEthernet->setChecked(false); // 取消勾选
            }
        }
    }
}

void BootloaderPage::onWiFiConnectClicked()
{
    if (isWiFiConnected) {
        // 断开WiFi
        if (wifiSocket) {
            wifiSocket->disconnectFromHost();
        }
        return;
    }

    // 连接WiFi
    QString ip = wifiIP->text();
    quint16 port = wifiPort->text().toUShort();
//连接逻辑:
//   - 获取IP地址和端口号
//   - 创建新的 QTcpSocket 对象
//   - 连接三个信号槽:
//     * connected -> onWiFiConnected
//     * disconnected -> onWiFiDisconnected
//     * readyRead -> onWiFiReadyRead
//   - 显示连接提示信息
//   - 调用 connectToHost() 发起连接
    wifiSocket = new QTcpSocket(this);
    connect(wifiSocket, &QTcpSocket::connected, this, &BootloaderPage::onWiFiConnected);
    connect(wifiSocket, &QTcpSocket::disconnected, this, &BootloaderPage::onWiFiDisconnected);
    connect(wifiSocket, &QTcpSocket::readyRead, this, &BootloaderPage::onWiFiReadyRead);

    receiveArea->appendPlainText(u8"[WiFi] 正在连接 " + ip + ":" + QString::number(port) + "...");
    wifiSocket->connectToHost(ip, port);
}

void BootloaderPage::onEthernetConnectClicked()
{
    if (isEthernetConnected) {
        // 断开以太网
        if (ethernetSocket) {
            ethernetSocket->disconnectFromHost();
        }
        return;
    }

    // 连接以太网
    QString ip = ethernetIP->text();
    quint16 port = ethernetPort->text().toUShort();

    ethernetSocket = new QTcpSocket(this);
    connect(ethernetSocket, &QTcpSocket::connected, this, &BootloaderPage::onEthernetConnected);
    connect(ethernetSocket, &QTcpSocket::disconnected, this, &BootloaderPage::onEthernetDisconnected);
    connect(ethernetSocket, &QTcpSocket::readyRead, this, &BootloaderPage::onEthernetReadyRead);

    receiveArea->appendPlainText(u8"[以太网] 正在连接 " + ip + ":" + QString::number(port) + "...");
    ethernetSocket->connectToHost(ip, port);
}

void BootloaderPage::onWiFiConnected()
{
    //receiveArea->appendPlainText(u8"[WiFi] 连接成功\n");
    btnWiFiConnect->setText(u8"断开");
    isWiFiConnected = true;

    // 启用功能按钮
    btnEraseRun->setEnabled(true);
    btnWriteAToRun->setEnabled(true);
    btnWriteBToRun->setEnabled(true);
    btnSendBin->setEnabled(true);
    btnGoRun->setEnabled(true);
    btnEnterShell->setEnabled(true);
    btnsendHelp->setEnabled(true);
    btnflashwriteToRun->setEnabled(true);
    btnflashwriteToA->setEnabled(true);
    btnflashwriteToB->setEnabled(true);
}

void BootloaderPage::onWiFiDisconnected()
{
    receiveArea->appendPlainText(u8"[WiFi] 连接已断开\n");
    btnWiFiConnect->setText(u8"连接");
    isWiFiConnected = false;
    useWiFi->setChecked(false);

    // 检查是否还有其他通信方式
    if (!useSerial->isChecked() && !useEthernet->isChecked()) {
        btnEraseRun->setEnabled(false);
        btnWriteAToRun->setEnabled(false);
        btnWriteBToRun->setEnabled(false);
        btnSendBin->setEnabled(false);
        btnGoRun->setEnabled(false);
        btnEnterShell->setEnabled(false);
        btnsendHelp->setEnabled(false);
        btnflashwriteToRun->setEnabled(false);
        btnflashwriteToA->setEnabled(false);
        btnflashwriteToB->setEnabled(false);
    }
}

void BootloaderPage::onEthernetConnected()
{
    receiveArea->appendPlainText(u8"[以太网] 连接成功\n");
    btnEthernetConnect->setText(u8"断开");
    isEthernetConnected = true;

    // 启用功能按钮
    btnEraseRun->setEnabled(true);
    btnWriteAToRun->setEnabled(true);
    btnWriteBToRun->setEnabled(true);
    btnSendBin->setEnabled(true);
    btnGoRun->setEnabled(true);
    btnEnterShell->setEnabled(true);
    btnsendHelp->setEnabled(true);
    btnflashwriteToRun->setEnabled(true);
    btnflashwriteToA->setEnabled(true);
    btnflashwriteToB->setEnabled(true);
}

void BootloaderPage::onEthernetDisconnected()
{
    receiveArea->appendPlainText(u8"[以太网] 连接已断开\n");
    btnEthernetConnect->setText(u8"连接");
    isEthernetConnected = false;
    useEthernet->setChecked(false);

    // 检查是否还有其他通信方式
    if (!useSerial->isChecked() && !useWiFi->isChecked()) {
        btnEraseRun->setEnabled(false);
        btnWriteAToRun->setEnabled(false);
        btnWriteBToRun->setEnabled(false);
        btnSendBin->setEnabled(false);
        btnGoRun->setEnabled(false);
        btnEnterShell->setEnabled(false);
        btnsendHelp->setEnabled(false);
        btnflashwriteToRun->setEnabled(false);
        btnflashwriteToA->setEnabled(false);
        btnflashwriteToB->setEnabled(false);
    }
}

void BootloaderPage::onWiFiReadyRead()
{
    if (!wifiSocket) return;

    QByteArray data = wifiSocket->readAll();

    // 如果sz进程在运行，将数据喂给它
//    if (szProcess && szProcess->state() == QProcess::Running) {
//        szProcess->write(data);
//        return;
//    }

    QScrollBar *vScrollBar = receiveArea->verticalScrollBar();
    bool isAtBottom = (vScrollBar->value() >= vScrollBar->maximum() - 5);

    QString str ;//= u8"[WiFi] "
    str += decoder ? decoder->toUnicode(data) : QString::fromLocal8Bit(data);

    receiveArea->moveCursor(QTextCursor::End);
    receiveArea->insertPlainText(str);

    if (isAtBottom) {
        receiveArea->ensureCursorVisible();
        vScrollBar->setValue(vScrollBar->maximum());
    }
}

void BootloaderPage::onEthernetReadyRead()
{
    if (!ethernetSocket) return;

    QByteArray data = ethernetSocket->readAll();

    if(data.isEmpty()) return;

// ★ 始终记录原始数据(十六进制)
//    QString hexData = data.toHex(' ').toUpper();
//    receiveArea->appendPlainText(QString("[以太网<-stm32]收到%1字节:%2")
//                                 .arg(data.size())
//                                 .arg(hexData));

    // 如果sz进程在运行，将数据喂给它
//    if (szProcess && szProcess->state() == QProcess::Running) {
//          szProcess->write(data);
//        return;
//    }

    //处理滚动条，让其在底部
    QScrollBar *vScrollBar = receiveArea->verticalScrollBar();
    bool isAtBottom = (vScrollBar->value() >= vScrollBar->maximum() - 5);


    QString str = decoder ? decoder->toUnicode(data) : QString::fromLocal8Bit(data);

    receiveArea->moveCursor(QTextCursor::End);
    receiveArea->insertPlainText(str);

    if (isAtBottom) {
        receiveArea->ensureCursorVisible();
        vScrollBar->setValue(vScrollBar->maximum());
    }
}

//专用于发送与shell进行交互的命令
void BootloaderPage::sendCommand(const QString &cmd)
{
    bool sent = false;

    // 串口发送
    if (useSerial->isChecked() && parentWidget) {
        QSerialPort *serialPort = parentWidget->getSerialPort();
        if (serialPort && serialPort->isOpen()) {
            serialPort->write(cmd.toLocal8Bit());
            serialPort->flush();
            //receiveArea->appendPlainText(cmd.trimmed());//u8"[] 发送: " + cmd.trimmed()
            sent = true;
        }
    }

    // WiFi发送
    if (useWiFi->isChecked() && wifiSocket && wifiSocket->state() == QTcpSocket::ConnectedState) {
        wifiSocket->write(cmd.toLocal8Bit());
        wifiSocket->flush();
        receiveArea->appendPlainText(u8"[WiFi]发送:" + cmd.trimmed());
        sent = true;
    }

    // 以太网发送
    if (useEthernet->isChecked() && ethernetSocket && ethernetSocket->state() == QTcpSocket::ConnectedState) {
        ethernetSocket->write(cmd.toLocal8Bit());
        ethernetSocket->flush();
        receiveArea->appendPlainText(u8"[以太网]发送:" + cmd.trimmed());
        sent = true;
    }

    if (!sent) {
        QMessageBox::warning(this, u8"提示", u8"请先选择并连接通信方式");
    }
}

void BootloaderPage::onEraseRunClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, u8"确认", u8"确定要擦除运行区(0x08009000)吗？",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QString cmd = "flash erase 0x08009000 0x20000\n";
        sendCommand(cmd);
    }
}

void BootloaderPage::onWriteAToRunClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, u8"确认", u8"确定要将A存储区内容写入运行区吗？",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QString cmd = "flash write 0x08029000 0x08009000 0x20000\n";
        sendCommand(cmd);
    }
}

void BootloaderPage::onWriteBToRunClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, u8"确认", u8"确定要将B存储区内容写入运行区吗？",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QString cmd = "flash write 0x08050000 0x08009000 0x20000\n";
        sendCommand(cmd);
    }
}

void BootloaderPage::onSendBinClicked()
{
    // 检查是否有可用的TCP连接（WiFi或以太网）
    QTcpSocket *activeSocket = nullptr;
    QString channelName;

    if (useWiFi->isChecked() && wifiSocket && wifiSocket->state() == QTcpSocket::ConnectedState) {
        activeSocket = wifiSocket;
        channelName = u8"WiFi";
    } else if (useEthernet->isChecked() && ethernetSocket && ethernetSocket->state() == QTcpSocket::ConnectedState) {
        activeSocket = ethernetSocket;
        channelName = u8"以太网";
    } else if (useSerial->isChecked() && parentWidget) {
        QSerialPort *serialPort = parentWidget->getSerialPort();
        if (serialPort && serialPort->isOpen()) {
            // 复用Widget的串口发送.bin功能
            parentWidget->triggerSendBin();
            return;
        }
    }

    if (!activeSocket) {
        QMessageBox::warning(this, u8"提示", u8"请先建立TCP连接（WiFi或以太网）");
        return;
    }

    if (szProcess && szProcess->state() == QProcess::Running) {
        QMessageBox::warning(this, u8"提示", u8"传输正在进行中");
        return;
    }

    if (fileTransferManager) {
        QMessageBox::warning(this, u8"提示", u8"传输正在进行中");
        return;
    }

    // 选择文件
    QString filePath = QFileDialog::getOpenFileName(this, u8"选择.bin文件", "", "*.bin");
    if (filePath.isEmpty())
        return;

    pendingFilePath = filePath;
    receiveArea->appendPlainText(u8"\n>>> 准备通过" + channelName + u8"发送: " + QFileInfo(filePath).fileName());
    // 先发送rz命令
//    receiveArea->appendPlainText(u8"请在目标设备执行 'rz' 命令...\n");
//    QString rzCmd = "rz\n";
//    QByteArray rzBytes = rzCmd.toLocal8Bit();
//    activeSocket->write(rzBytes);
//    activeSocket->flush();
//    // 等待2秒后启动传输，等着的2s是等stm32端的rz恢复ACK，等待接收
//    QTimer::singleShot(2000, this, &BootloaderPage::startZmodemTransfer);

//    //创建传输管理器前，断开界面的readyRead连接
    if(activeSocket == wifiSocket){
        disconnect(wifiSocket,&QTcpSocket::readyRead,this,&BootloaderPage::onWiFiReadyRead);
    }
    else{
        disconnect(ethernetSocket,&QTcpSocket::readyRead,this,&BootloaderPage::onEthernetReadyRead);
    }

    // 创建传输管理器
    fileTransferManager = new FileTransferManager(activeSocket, this);

    connect(fileTransferManager, &FileTransferManager::transferStarted,
            this, [this](const QString &fileName, uint32_t fileSize) {
        receiveArea->appendPlainText(QString(u8"[传输] 开始发送: %1 (%2 字节)")
                                     .arg(fileName).arg(fileSize));
    });
    //FileTransferManager::onSocketReadyRead() 中收到设备的 READY 帧后--
    //"emit transferStarted(QFileInfo(file).fileName(), totalFileSize);"--触发
    //作用：向界面输出传输开始的提示信息，显示文件名和大小

    connect(fileTransferManager, &FileTransferManager::transferProgress,
            this, [this](uint32_t sent, uint32_t total) {
        receiveArea->appendPlainText(QString(u8"[传输] 进度: %1 / %2 字节")
                                     .arg(sent).arg(total));
    });
    //每次收到设备的 ACK 帧后（表示一块数据已成功接收）
    //emit transferProgress(sentBytes, totalFileSize);--触发
    //作用：实时显示传输进度

    connect(fileTransferManager, &FileTransferManager::transferFinished,
            this, [this](bool , const QString &message) {
        receiveArea->appendPlainText(u8"[传输] " + message);
        fileTransferManager->deleteLater();
        fileTransferManager = nullptr;
    });
    //所有数据块发送完毕，发送 FILE_END 帧后--触发
    //deleteLater() - 延迟删除 FileTransferManager 对象（安全释放资源）

    if(activeSocket == wifiSocket){
        connect(wifiSocket,&QTcpSocket::readyRead,this,&BootloaderPage::onWiFiReadyRead);
    }
    else{
        connect(ethernetSocket,&QTcpSocket::readyRead,this,&BootloaderPage::onEthernetReadyRead);
    }


    connect(fileTransferManager, &FileTransferManager::error,
            this, [this](const QString &errMsg) {
        receiveArea->appendPlainText(u8"[传输错误] " + errMsg);
        fileTransferManager->deleteLater();
        fileTransferManager = nullptr;
    });
    //传输过程中出现错误时触发
    //清理资源

    if(activeSocket == wifiSocket){
        connect(wifiSocket,&QTcpSocket::readyRead,this,&BootloaderPage::onWiFiReadyRead);
    }
    else{
        connect(ethernetSocket,&QTcpSocket::readyRead,this,&BootloaderPage::onEthernetReadyRead);
    }

    fileTransferManager->sendFile(filePath);

}

/*
void BootloaderPage::startZmodemTransfer()
{
    if (pendingFilePath.isEmpty())
        return;

    // 确定使用哪个socket
    QTcpSocket *activeSocket = nullptr;
    if (useWiFi->isChecked() && wifiSocket && wifiSocket->state() == QTcpSocket::ConnectedState) {
        activeSocket = wifiSocket;
    } else if (useEthernet->isChecked() && ethernetSocket && ethernetSocket->state() == QTcpSocket::ConnectedState) {
        activeSocket = ethernetSocket;
    }

    if (!activeSocket) {
        QMessageBox::warning(this, u8"错误", u8"TCP连接已断开");
        pendingFilePath.clear();
        return;
    }

    receiveArea->appendPlainText(u8">>> 启动 Z-modem 传输...\n");

    szProcess = new QProcess(this);
    // sz的输出重定向到TCP Socket
    connect(szProcess, &QProcess::StandardOutput, this, [this, activeSocket]() {
        QByteArray data = szProcess->readAllStandardOutput();

        //调试信息,Qt向32发送的数据(传输.bin文件阶段)
//        QString hexData = data.toHex(' ').toUpper();
//        receiveArea->appendPlainText(QString("收到%1字节:%2")
//                                     .arg(data.size())
//                                     .arg(hexData));

        if (activeSocket && activeSocket->state() == QTcpSocket::ConnectedState) {
                activeSocket->write(data);
          }else {
            receiveArea->appendPlainText(QString("[错误] TCP连接已断开!"));
        }
    });

    // sz的错误输出显示到界面
    connect(szProcess, &QProcess::StandardError, this, [this]() {
        QString err = QString::fromLocal8Bit(szProcess->readAllStandardError());
        receiveArea->appendPlainText(u8"[sz错误] " + err);
    });

    // 传输完成处理
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

    // 构建sz命令
    QString program = "sz";
    QStringList arguments;
    arguments << "-b"               // 二进制模式
              << "-v"               // 详细输出
              << pendingFilePath;   // 文件路径

#ifdef Q_OS_WIN
       program = "sz";
#endif
    szProcess->start(program, arguments);

    if (!szProcess->waitForStarted(3000)) {
        receiveArea->appendPlainText(u8"错误: 无法启动 sz 工具\n");
        receiveArea->appendPlainText(u8"请确保已安装lezsz并在PATH中\n");
        szProcess->deleteLater();
        szProcess = nullptr;
    }
}*/

void BootloaderPage::onGoRunClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, u8"确认", u8"确定要跳转到运行区(0x08009000)吗？",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QString cmd = "go\n";
        sendCommand(cmd);
    }
}


void BootloaderPage::onSendHelpClicked()
{
    QString cmd = "help\n"; // 发送空格+换行
    sendCommand(cmd);
}

void BootloaderPage::FlashwriteToRun()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, u8"确认", u8"确定要将内存0x20000000的内容写入运行区吗？",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes){
        QString cmd1 = "flash erase 0x08009000 0x20000\n"; // 发送空格+换行
        sendCommand(cmd1);
        // 2秒后自动执行 Lambda 表达式里的代码
        QTimer::singleShot(2000, this, [=](){
            QString cmd2 = "flash write 0x20000000 0x08009000 0x20000\n";
            sendCommand(cmd2);
        });
    }
}

void BootloaderPage::FlashwriteToAppA()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, u8"确认", u8"确定要将内存0x20000000的内容写入A存储区吗？",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes){
        QString cmd1 = "flash erase 0x08029000 0x20000\n"; // 发送空格+换行
        sendCommand(cmd1);
        // 2秒后自动执行 Lambda 表达式里的代码
        QTimer::singleShot(2000, this, [=](){
            QString cmd2 = "flash write 0x20000000 0x08029000 0x20000\n";
            sendCommand(cmd2);
        });
     }
}

void BootloaderPage::FlashwriteToAppB()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, u8"确认", u8"确定要将内存0x20000000的内容写入B存储区吗？",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes){
        QString cmd1 = "flash erase 0x08050000 0x20000\n"; // 发送空格+换行
        sendCommand(cmd1);
        // 2秒后自动执行 Lambda 表达式里的代码
        QTimer::singleShot(2000, this, [=](){
            QString cmd2 = "flash write 0x20000000 0x08050000 0x20000\n";
            sendCommand(cmd2);
        });
    }
}

void BootloaderPage::onEnterShellClicked()
{
    QString cmd = " \n"; // 发送空格+换行
    sendCommand(cmd);
    receiveArea->appendPlainText(u8">>> 已发送空格键（进入Shell）\n");
}

void BootloaderPage::appendSerialData(const QString &data)
{
    // 从Widget接收串口数据并显示
    QScrollBar *vScrollBar = receiveArea->verticalScrollBar();
    bool isAtBottom = (vScrollBar->value() >= vScrollBar->maximum() - 5);

    QString str =  data;//u8"[串口] " +data

    receiveArea->moveCursor(QTextCursor::End);
    receiveArea->insertPlainText(str);

    if (isAtBottom) {
        receiveArea->ensureCursorVisible();
        vScrollBar->setValue(vScrollBar->maximum());
    }
}

//在widget.cpp中调用，在连接串口时调用一下。
void BootloaderPage::updateSerialStatus(bool connected)
{
    useSerial->setEnabled(connected);
    if (connected) {
        lblSerialPort->setText(u8"端口: " + (parentWidget ? parentWidget->getCurrentPortName() : u8"已连接"));
        lblBaudRate->setText(u8"波特率: " + (parentWidget ? parentWidget->getCurrentBaudRate() : u8"--"));
        useSerial->setChecked(true);

        // 启用功能按钮
        btnEraseRun->setEnabled(true);
        btnWriteAToRun->setEnabled(true);
        btnWriteBToRun->setEnabled(true);
        btnSendBin->setEnabled(true);
        btnGoRun->setEnabled(true);
        btnEnterShell->setEnabled(true);
        btnsendHelp->setEnabled(true);
        btnflashwriteToRun->setEnabled(true);
        btnflashwriteToA->setEnabled(true);
        btnflashwriteToB->setEnabled(true);
    } else {
        lblSerialPort->setText(u8"端口: 未连接");
        lblBaudRate->setText(u8"波特率: --");
        useSerial->setChecked(false);

        // 检查是否还有其他通信方式
        if (!useWiFi->isChecked() && !useEthernet->isChecked()) {
            btnEraseRun->setEnabled(false);
            btnWriteAToRun->setEnabled(false);
            btnWriteBToRun->setEnabled(false);
            btnSendBin->setEnabled(false);
            btnGoRun->setEnabled(false);
            btnEnterShell->setEnabled(false);
            btnsendHelp->setEnabled(false);
            btnflashwriteToRun->setEnabled(false);
            btnflashwriteToA->setEnabled(false);
            btnflashwriteToB->setEnabled(false);
        }
    }
}

void BootloaderPage::updateSerialConfig(const QString &portName, const QString &baudRate)
{
    lblSerialPort->setText(u8"端口: " + portName);
    lblBaudRate->setText(u8"波特率: " + baudRate);
}

void BootloaderPage::cleanup()
{
    // 清理sz进程
    if (szProcess) {
        if (szProcess->state() == QProcess::Running) {
            szProcess->terminate();
            szProcess->waitForFinished(1000);
        }
        szProcess->deleteLater();
        szProcess = nullptr;
    }

    // 清理WiFi连接
    if (wifiSocket) {
        wifiSocket->disconnectFromHost();
        wifiSocket->deleteLater();
        wifiSocket = nullptr;
    }

    // 清理以太网连接
    if (ethernetSocket) {
        ethernetSocket->disconnectFromHost();
        ethernetSocket->deleteLater();
        ethernetSocket = nullptr;
    }

    // TCP传输.bin文件进程
    if (fileTransferManager) {
        fileTransferManager->stopTransfer();
        fileTransferManager->deleteLater();
        fileTransferManager = nullptr;
    }
}
