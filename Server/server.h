//============================================================================
// Name        : server.h
// Author      : xiaoming
// Version     :
// Date        : 2013.07.05
// Copyright   : xiaoming_beta@163.com
// Description : the server function class
//============================================================================

#ifndef SERVER_H
#define SERVER_H
#include "udpsockets.h"

#include <QMainWindow>
#include <QStandardItemModel>
#include <QTimer>
#include <QDateTime>
#include <Qstring>
#include <QFileDialog>
#include <QMessageBox>
namespace Ui {
class Server;
}

class Server : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit Server(QWidget *parent = 0);
    virtual ~Server();

    bool fillMsg(QStringList recvMsg);
    void displayMsg();

private slots:
    void on_checkBox_stateChanged(int state);
    void recvMsg();
    void sendMsg();
signals:
    void sendMsgEmit();
private:
    Ui::Server *ui;
    QStringList m_recvData;
    QString m_port;
    QString m_id;

    UdpSockets m_socket;
    QStandardItemModel *m_modelMsg;
    static int m_recvMsgRight;
    static int m_recvMsgError;
    static int m_sendMsgSum;

};

#endif // SERVER_H
