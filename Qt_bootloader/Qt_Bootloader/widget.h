#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QDebug>
#include <QSerialPort>
#include <QSerialPortInfo> //用于获取可用串口列表
#include <QTimerEvent>
#include <algorithm>
#include <QMessageBox>//提示弹窗
#include <QTextDecoder>
#include <QTextCodec>
#include <QCheckBox>
#include <QScrollBar>
#include <QFileDialog>
#include <QProcess>
#include <QTimer>
#include <QStackedWidget>
#include <QGroupBox>
#include "networkpage.h"
#include "bootloaderpage.h"

class BootloaderPage;//添加bootloader页面的前向声明

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void ReceiveAreaInit(void);//接收区初始化------弃用
    void SendAreaInit(void);//发送区初始化------弃用
    void SetupInit(void);//设置下拉框初始化------弃用
    void beginUART(void);//连接串口按钮、对应信号槽连接------弃用

    void SetupUiInit(); //新的UI界面,取代ReceiveAreaInit、SendAreaInit、SetupInit、beginUART
    void USART(void);//专用于连接的函数
    void timerEvent(QTimerEvent* e);//定时刷新串口号下拉框
    void HandleSerialData();     // 统一处理串口接收

    //版3--添加访问接口
public:
    QSerialPort *getSerialPort(){ return serialPort;}//获取串口‘实例’
    QString getCurrentPortName(){ return portNumber?portNumber->currentText():"";}//获取当前串口号
    QString getCurrentBaudRate(){ return baudRate?baudRate->currentText():"";}//获取当前波特率
    void triggerSendBin();//触发发送.bin文件功能

public slots:
    void on_btnSelectAndSendBin_clicked();
    void startSzTransfer();  // 启动 sz 传输

    void switchToNetwork();         // 切换到网络页
    void switchToSerial();          // 返回串口页
    void switchToBootloader();      // 切换到Bootloader页--版3
    void switchToSerialFromBootloader();//从Bootloader返回串口页面--版3

    //版3--添加信号
signals:
    void serialDataReceived(const QString &data);//串口数据接收信号
    void serialConfigChanged(const QString &portName,const QString &baudRate);//串口配置变更信号
    void serialConnected(bool connected);//串口连接状态信号


private:
    void initPages();
    void initSerialPage();          // 初始化串口页

    QStackedWidget *stackedWidget;  // 页面容器
    QWidget *serialPage;            // 串口页（原主页）
    NetworkPage *networkPage;       // 【新增】网络页
    //BootloaderControlPage *bootloaderpage;//【新增】bootloader命令页--版1
    BootloaderPage *bootloaderPage; //版2、3一致

private:
    QPlainTextEdit* sendArea;//发送区
    QPlainTextEdit* receivedArea;//接收区

    QPushButton* sendButton;//发送按钮
    QPushButton* Estbconnect;//建立连接
    QPushButton* Disconnect;//断开连接
    QPushButton* sendBinBtn;//发送.bin文件按钮

    QCheckBox* addNewLine;//发送新行复选框

    QComboBox* portNumber;//端口-下拉可选框
    QComboBox* baudRate;
    QComboBox* dataSize;
    QComboBox* stopSize;
    QComboBox* check;
    QComboBox* receiveMode;
    QComboBox* sendMode;

    QSerialPort* serialPort;
    QVector<QString>ports;

    QTextCodec *codec;
    QTextDecoder *decoder;

    QProcess *szProcess = nullptr;//sz进程
    QString pendingFilePath;//待发送文件路径

};


#endif // WIDGET_H
