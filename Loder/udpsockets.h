//============================================================================
// Name        : updsocket.h
// Author      : xiaoming
// Version     :
// Date        : 2013.07.05
// Copyright   : xiaoming_beta@163.com
// Description : the upd socket
//============================================================================

#ifndef UDPSOCKETS_H
#define UDPSOCKETS_H

#include <QObject>
#include <QUdpSocket>
#include <QStringList>

#include "headFile.h"
#include "msg.h"

using namespace std;
class UdpSockets : public QObject
{
    Q_OBJECT

public:
    explicit UdpSockets(QObject *parent = 0);
    virtual ~UdpSockets();

    void initSocket(bool isServer);
    void connectOther(int port);
    QStringList parseMsg(t_msg msg);
    void sendMsg(QStringList qMsg, QString recvIp, unsigned port);
    void recvMsgBind();
    void closeSocket();

    inline void setClientIp(string ip)
    {
        m_clientIp = ip;
    }
    inline string getClientIp()
    {
        return m_clientIp;
    }
    inline void setPort(unsigned int port)
    {
        m_port = port;
    }
    inline unsigned int getPort()
    {
        return m_port;
    }
    inline QByteArray getRecvMsg()
    {
        return m_recvMsg;
    }
    inline bool isServer()
    {
        return m_isServer;
    }
private slots:
    void readPendingDatagrams();
signals:
    void recvData();
    void recvServerData();
private:
    string m_clientIp;
    string m_loadIp;
    bool m_isServer;
    unsigned int m_port;
    QUdpSocket *m_socket;
    QByteArray m_recvMsg;

};

#endif // UDPSOCKETS_H
