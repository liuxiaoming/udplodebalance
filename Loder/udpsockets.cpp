#include "udpsockets.h"

UdpSockets::UdpSockets(QObject *parent) :
    QObject(parent)
{
    m_port = 0;
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
void UdpSockets::initSocket(bool isServer)
{
    m_isServer = isServer;
    m_socket = new QUdpSocket(this);
    connect(m_socket,SIGNAL(disconnected()),m_socket,
               SLOT(deleteLater()));
}

/*
 *connectOther
 *负载均衡器打开的时候调用，用来连接UDP端口，打开收发数据端口
 */
void UdpSockets::connectOther(int port)
{
    m_port = port;
    if(m_socket == NULL)
        m_socket = new QUdpSocket(this);
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
    m_port = port;
    QByteArray data;    //字节数组
    QString ip = QString::fromStdString(m_clientIp);
    for(int i = 0; i < qMsg.length(); ++i)
    {
        data.append(qMsg.at(i));
        data.append("$");
    }
    data.append("loder");
    //一个udpSocket已经于一个端口bind在一起了，这里的data是out流中的data，最多可以传送8192个字节，但是建议不要超过
    //512个字节，因为这样虽然可以传送成功，但是这些数据需要在ip层分组，QHostAddress::Broadcast是指发送数据的目的地址为广播，也会给本机发送，然后本机会接收，然后再发送，形成死循环
    //这里为本机所在地址的广播组内所有机器，即局域网广播发送
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
    QHostAddress *serverHost = new QHostAddress();
    quint16 *port = new quint16();
    m_recvMsg.clear();
    while (m_socket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_socket->pendingDatagramSize());
        m_socket->readDatagram(datagram.data(), datagram.size(), serverHost, port);

        QStringList tmp = QString(datagram).split("$");
        //数据接收完毕后，发送数据接收完毕信号
        if(tmp.at(tmp.length() - 1) == "client")
        {
            m_recvMsg = datagram;
            emit recvData();
        }
        else if(tmp.at(tmp.length() - 1) == "server")
        {
            m_recvMsg = datagram;
            emit recvServerData();
        }
    }
}


/*
 *closeSocket
 *负载均衡器调试开关，关闭时候调用，关闭掉socket，不接受任何数据。
 */
void UdpSockets::closeSocket()
{
//    if(m_socket->isOpen())
    m_socket->close();
    if(m_socket != NULL)
    {
        delete m_socket;
        m_socket = NULL;
    }
}
