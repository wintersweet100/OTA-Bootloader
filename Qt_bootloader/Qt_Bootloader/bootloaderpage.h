#ifndef BOOTLOADERPAGE_H
#define BOOTLOADERPAGE_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QGroupBox>
#include <QCheckBox>
#include <QTcpSocket>
#include <QProcess>
#include <QTextDecoder>
#include <QThread>
#include "filetransfermanager.h"

// 前向声明
class Widget;

class BootloaderPage : public QWidget
{
    Q_OBJECT

public:
    explicit BootloaderPage(Widget *parentWidget, QWidget *parent = nullptr);
    ~BootloaderPage();

    // 串口数据接收接口
    void appendSerialData(const QString &data);

    // 串口连接状态更新
    void updateSerialStatus(bool connected);

    // 更新串口配置显示
    void updateSerialConfig(const QString &portName, const QString &baudRate);

signals:
    void backToMain();

private slots:
    // 返回按钮
    void onBackClicked();

    // 通信方式选择
    void onSerialCheckChanged(int state);
    void onWiFiCheckChanged(int state);
    void onEthernetCheckChanged(int state);

    // 功能按钮
    void onEraseRunClicked();      // 擦除运行区
    void onWriteAToRunClicked();   // A区→运行区
    void onWriteBToRunClicked();   // B区→运行区
    void onSendBinClicked();       // 发送.bin文件
    void onGoRunClicked();         // 跳转到运行区
    void onEnterShellClicked();    // 进入Shell
    void onSendHelpClicked();       // 发送help
    void FlashwriteToRun();         // 将内存数据复制到运行区
    void FlashwriteToAppA();        // 将内存数据复制到A存储区
    void FlashwriteToAppB();        // 将内存数据复制到B存储区

    // 网络连接
    void onWiFiConnectClicked();
    void onEthernetConnectClicked();

    // 网络数据接收
    void onWiFiReadyRead();
    void onEthernetReadyRead();
    void onWiFiConnected();
    void onWiFiDisconnected();
    void onEthernetConnected();
    void onEthernetDisconnected();

private:
    void initUI();
    void sendCommand(const QString &cmd);
    void startZmodemTransfer();
    void cleanup();

    // UI组件
    QPushButton *btnBack;

    // 接收区（统一显示）
    QPlainTextEdit *receiveArea;
    QPushButton *btnClearRecv;

    // 串口配置区
    QLabel *lblSerialPort;
    QLabel *lblBaudRate;
    QCheckBox *useSerial;

    // WiFi配置区
    QLineEdit *wifiIP;
    QLineEdit *wifiPort;
    QCheckBox *useWiFi;
    QPushButton *btnWiFiConnect;
    bool isWiFiConnected;

    // 以太网配置区
    QLineEdit *ethernetIP;
    QLineEdit *ethernetPort;
    QCheckBox *useEthernet;
    QPushButton *btnEthernetConnect;
    bool isEthernetConnected;

    // 地址显示
    QLabel *lblRunAddr;
    QLabel *lblAAddr;
    QLabel *lblBAddr;

    // 功能按钮
    QPushButton *btnEraseRun;
    QPushButton *btnWriteAToRun;
    QPushButton *btnWriteBToRun;
    QPushButton *btnSendBin;
    QPushButton *btnGoRun;
    QPushButton *btnEnterShell;
    QPushButton *btnsendHelp;
    QPushButton *btnflashwriteToRun;
    QPushButton *btnflashwriteToA;
    QPushButton *btnflashwriteToB;

    // 网络通信
    QTcpSocket *wifiSocket;
    QTcpSocket *ethernetSocket;

    // Z-modem传输
    QProcess *szProcess;
    QString pendingFilePath;

    // 文本解码器
    QTextDecoder *decoder;

    // 父窗口引用（用于访问串口）
    Widget *parentWidget;

    FileTransferManager *fileTransferManager = nullptr;
};

#endif
