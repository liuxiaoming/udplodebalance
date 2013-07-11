#include "server.h"
#include "ui_server.h"
int Server::m_recvMsgRight = 0;
int Server::m_recvMsgError = 0;
int Server::m_sendMsgSum = 0;

Server::Server(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Server)
{
    ui->setupUi(this);

    //初始化消息显示面板，设置字段等。
    m_modelMsg = new QStandardItemModel(this);
    m_modelMsg->setHorizontalHeaderItem(0, new QStandardItem(QObject::tr("消息类型")));
    m_modelMsg->setHorizontalHeaderItem(1, new QStandardItem(QObject::tr("des_id")));
    m_modelMsg->setHorizontalHeaderItem(2, new QStandardItem(QObject::tr("消息内容")));
    m_modelMsg->setHorizontalHeaderItem(3, new QStandardItem(QObject::tr("收发时间")));
    ui->tableView->setModel(m_modelMsg);

    m_socket.initSocket();
    connect(&m_socket, SIGNAL(recvData()), this, SLOT(recvMsg()));//接收客户端发送的UDP数据消息
    connect(this, SIGNAL(sendMsgEmit()), this, SLOT(sendMsg()));
}

Server::~Server()
{
    delete ui;
    if(m_modelMsg != NULL)
    {
        delete m_modelMsg;
        m_modelMsg = NULL;
    }
}

/*
 *recvMsg
 *服务器接收负载均衡器发送的消息，接收完毕后，发送sendMsgEmit信号，通过信号发送应答消息给负载均衡器
 */
void Server::recvMsg()
{
    m_socket.recvMsgBind();
    QStringList recvMsg = QString(m_socket.getRecvMsg()).split("$");
    if(fillMsg(recvMsg))
    {
        ++m_recvMsgRight;
        ui->lineEdit_RecvMsgRight->setText(QString("%1").arg(m_recvMsgRight));
        ui->lineEdit_RecvMsgSum->setText(QString("%1").arg(m_recvMsgError + m_recvMsgRight));

        m_recvData = recvMsg;
        emit sendMsgEmit();
    }
}

/*
 *fillMsg
 *将负载均衡器接收或发送的消息，经过格式转后后，填入消息显示面板
 */
bool Server::fillMsg(QStringList recvMsg)
{
    if(recvMsg.at(1) != m_id)
        return false;
    QString src_id = recvMsg.at(2);
    QString msgType = "未知";
    switch(recvMsg.at(3).toInt())
    {
    case 0:
        msgType = "时间请求";
        break;
    case 1:
        msgType = "时间答应";
        break;
    case 2:
        msgType = "心跳请求";
        break;
    case 3:
        msgType = "心跳应答";
        break;
    }

    QString msgContent = recvMsg.at(4);
    QString msgTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    int rowCount = m_modelMsg->rowCount();
    m_modelMsg->setItem(rowCount, 0, new QStandardItem(msgType));
    m_modelMsg->setItem(rowCount, 1, new QStandardItem(src_id));
    m_modelMsg->setItem(rowCount, 2, new QStandardItem(msgContent));
    m_modelMsg->setItem(rowCount, 3, new QStandardItem(msgTime));


    return true;
}
/*
 *on_checkBox_stateChanged
 *槽函数，响应调试开关checkBox的函数
 */
void Server::on_checkBox_stateChanged(int state)
{
    m_id = ui->lineEdit_ID->text();
    m_port = ui->lineEdit_UdpPort->text();
    if(state == 2)
    {
        if(m_id == "" || m_port == "")
        {
            QMessageBox::warning(this, tr("警告"),
                                 tr("为填入初始化信息，请填入ID和端口号"),
                                 QMessageBox::Ok);
            ui->checkBox->setCheckState(Qt::Unchecked);
            return;
        }
    }
    displayMsg();
}

/*
 *displayMsg
 *由调试开关的状态选择是否打开端口收发数据
 */
void Server::displayMsg()
{
    if(ui->checkBox->checkState() == Qt::Checked)
    {
        m_socket.connectOther(m_port.toInt());
        ui->tabWidget->setCurrentIndex(1);
    }
    else if(ui->checkBox->checkState() == Qt::Unchecked)
    {
        m_socket.closeSocket();
        ui->tabWidget->setCurrentIndex(0);
    }
}

/*
 *sendMsg
 *服务器发送消息的槽函数，通过信号sendMsgEmit信号来调用
 */
void Server::sendMsg()
{
    QStringList qServerMsg;
    qServerMsg << m_id << m_recvData.at(2) << m_id << "1" << ui->lineEdit_Msg->text(); //
    m_socket.sendMsg(qServerMsg, QString::fromStdString(""), m_port.toInt());
    ++m_sendMsgSum;
    ui->lineEdit_SendMsgSum->setText(QString("%1").arg(m_sendMsgSum));
    qServerMsg.clear();
    qServerMsg << m_id << m_id << m_recvData.at(2) << "1" << ui->lineEdit_Msg->text();
    fillMsg(qServerMsg);
}
