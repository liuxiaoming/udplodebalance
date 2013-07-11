//============================================================================
// Name        : loder.h
// Author      : xiaoming
// Version     :
// Date        : 2013.07.04
// Copyright   : xiaoming_beta@163.com
// Description : the loder function class
//============================================================================

#ifndef LODER_H
#define LODER_H
#include "udpsockets.h"

#include <QMainWindow>
#include <QStandardItemModel>
#include <QTimer>
#include <QDateTime>
#include <Qstring>
#include <QFileDialog>
#include <QMutex>

namespace Ui {
class Loder;
}

class Loder : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit Loder(QWidget *parent = 0);
    virtual ~Loder();
    QStringList readFile(QString fileName);
    void configuration(QStringList list);
    void writeLog(QStringList msg);
    void fillMsg(QStringList recvMsg, bool RorS);
    void sendMsgToServer(QStringList qMsg);
    void sendMsgToClient(QStringList qMsg);

private slots:
    void openFilePath();
    void displayMsg();
    void displayLog();
    void addClientMsgToTable();
    void addServerMsgToTable();

private:
    Ui::Loder *ui;
    QString m_loaderID;
    QString m_clientPort;
    QString m_serverPort;

    QTimer *m_timer;
    UdpSockets m_udpClientSocket;//客户端udp socket
    UdpSockets m_udpServerSocket;//服务器端udp socket

    //map对应着服务器的id 和 端口号
    QString m_serverNum;
    map<QString, QString> m_server;
    QStandardItemModel *m_modelServer;
    QStandardItemModel *m_modelMsg;
    QStandardItemModel *m_modelLog;
    static int currentServer;

    static int m_clientRecvErrorMsg;
    static int m_clientRecvRightMsg;
    static int m_clentSendMsgSum;
    static int m_serverRecvErrorMsg;
    static int m_serverRecvRightMsg;
    static int m_serverSendMsgSum;
};

#endif // LODER_H
