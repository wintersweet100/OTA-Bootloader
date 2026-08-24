#ifndef NETWORKPAGE_H
#define NETWORKPAGE_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QTcpSocket>
#include <QTcpServer>
#include <QUdpSocket>
#include <QTextCodec>
#include <QScrollBar>
#include <QProcess>
#include <QFileDialog>
#include <QTimer>
#include <QCoreApplication>
#include <QCheckBox>

class NetworkPage : public QWidget
{
    Q_OBJECT
public:
    explicit NetworkPage(QWidget *parent = nullptr);
    ~NetworkPage();

signals:
    void backToMain();  // 返回主界面信号

private slots:
    void onConnectClicked();      // 连接/断开按钮
    void onSendClicked();         // 发送数据
    void onTcpConnected();        // TCP 连接成功
    void onTcpDisconnected();     // TCP 断开
    void onTcpReadyRead();        // TCP 接收数据
    void onUdpReadyRead();        // UDP 接收数据
    void onProtocolChanged(int index);  // 协议切换
    void onSendBinClicked();    //发送.bin文件

private:
    void initUI();
    void cleanup();
    void startZmodemTransfer();

    // UI 控件
    QPlainTextEdit *receiveArea;   // 接收区
    QPlainTextEdit *sendArea;      // 发送区

    QComboBox *protocolType;       // 协议类型(TCP/UDP)
    QComboBox *clientType;         // 客户端/服务器
    QLineEdit *ipAddress;          // IP地址
    QLineEdit *port;               // 端口号
    QComboBox *receiveMode;        // 接收模式(HEX/文本)
    QComboBox *sendMode;           // 发送模式

    QCheckBox *addNexLine;         //复选框:发送新行

    QPushButton *btnConnect;       // 连接按钮
    QPushButton *btnSend;          // 发送按钮
    QPushButton *btnClearRecv;     // 清空接收区
    QPushButton *btnClearSend;     // 清空发送区
    QPushButton *btnBack;          // 返回按钮
    QPushButton *btnSendBin;       //发送.bin按钮

    // 网络对象
    QTcpSocket *tcpSocket;         // TCP客户端
    QTcpServer *tcpServer;         // TCP服务器
    QUdpSocket *udpSocket;         // UDP

    QProcess *szProcess = nullptr;
    QString pendingFilePath;

    QTextDecoder *decoder;         // 文本解码器
    bool isConnected;              // 连接状态
};

#endif // NETWORKPAGE_H
