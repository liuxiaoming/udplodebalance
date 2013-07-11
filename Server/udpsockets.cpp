#include "udpsockets.h"

UdpSockets::UdpSockets(QObject *parent) :
    QObject(parent)
{
    m_socket = NULL;
}
UdpSockets::~UdpSockets()
{
    if(m_socket != NULL)
    {
        closeSocket();
        delete m_socket;
    }
}

/*
 *initSocket
 *初始化udp socket
 */
void UdpSockets::initSocket()
{
    m_socket = new QUdpSocket(this);
//    connect(m_socket, SIGNAL(disconnected()), this,
//               SLOT(deleteLater()));
}

/*
 *connectOther
 *负载均衡器打开的时候调用，用来连接UDP端口，打开收发数据端口
 */
void UdpSockets::connectOther(int port)
{
    if(m_socket->state() == QAbstractSocket::UnconnectedState)
        m_socket->bind(port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);//端口绑定
    connect(m_socket, SIGNAL(readyRead()),
    this, SLOT(readPendingDatagrams()));//readyRead()信号是每当有新的数据来临时就被触发
}

QStringList UdpSockets::parseMsg(t_msg msg)
{
    QStringList list;
    list << QString::number(msg.src_id) << QString::number(msg.dst_id) << QString::number(msg.usr_id)
         << QString::number(msg.msg_type, 10) << QString(msg.data);

    return list;
}

//发送的数据位qMsg，发送给ip地址为：recvIp,如果recvIp字段为空，则为广播发送
void UdpSockets::sendMsg(QStringList qMsg, QString recvIp, unsigned port)
{
    QByteArray data;    //字节数组
    QString ports = QString("%1").arg(port);
    data.clear();
    for(int i = 0; i < qMsg.length(); ++i)
    {
        data.append(qMsg.at(i));
        data.append("$");
    }
    data.append(ports);
    data.append("$server");
    if(recvIp.length() == 0)
        m_socket->writeDatagram(data, data.length(), QHostAddress::Broadcast, port);//将data中的数据发送
    else
        m_socket->writeDatagram(data, data.length(), QHostAddress(recvIp), port);//将data中的数据发送
}

void UdpSockets::recvMsgBind()
{

}

/*
 *readPendingDatagrams
 *接受并读取发送的数据，将数据存放在m_recvMsg中
 */
void UdpSockets::readPendingDatagrams()
{
    m_recvMsg.clear();
    while (m_socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_socket->pendingDatagramSize());
        m_socket->readDatagram(datagram.data(), datagram.size());
        QStringList tmp = QString(datagram).split("$");
        if(tmp.at(tmp.length() - 1) == "loder")
        {
            m_recvMsg = datagram;
            emit recvData();
        }
    }

}

/*
 *closeSocket
 *负载均衡器调试开关，关闭时候调用，关闭掉socket，不接受任何数据。
 */
void UdpSockets::closeSocket()
{
    m_socket->close();
}
