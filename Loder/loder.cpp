#include "loder.h"
#include "ui_loder.h"
#include <QTextCodec>
int Loder::currentServer = -1;
int Loder::m_clientRecvErrorMsg = 0;
int Loder::m_clientRecvRightMsg = 0;
int Loder::m_serverRecvErrorMsg = 0;
int Loder::m_serverRecvRightMsg = 0;
int Loder::m_clentSendMsgSum = 0;
int Loder::m_serverSendMsgSum = 0;

Loder::Loder(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Loder)
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
    ui->setupUi(this);
    ui->Button_Switch->setText(tr("调试(close)"));
    ui->Button_Switch->setEnabled(false);
    m_timer = new QTimer(this);

    //初始化服务器配置面板，设置字段等。
    m_modelServer = new QStandardItemModel(this);
    m_modelServer->setHorizontalHeaderItem(0, new QStandardItem(QObject::tr("服务器ID")));
    m_modelServer->setHorizontalHeaderItem(1, new QStandardItem(QObject::tr("服务器UDP端口")));
    ui->tableView_Server->setModel(m_modelServer);

    //初始化消息显示面板，设置字段等。
    m_modelMsg = new QStandardItemModel(this);
    m_modelMsg->setHorizontalHeaderItem(0, new QStandardItem(QObject::tr("消息类型")));
    m_modelMsg->setHorizontalHeaderItem(1, new QStandardItem(QObject::tr("消息来源")));
    m_modelMsg->setHorizontalHeaderItem(2, new QStandardItem(QObject::tr("src_id")));
    m_modelMsg->setHorizontalHeaderItem(3, new QStandardItem(QObject::tr("des_id")));
    m_modelMsg->setHorizontalHeaderItem(4, new QStandardItem(QObject::tr("消息内容")));
    m_modelMsg->setHorizontalHeaderItem(5, new QStandardItem(QObject::tr("收发日期")));
    ui->tableView_Msg->setModel(m_modelMsg);

    m_modelLog = new QStandardItemModel(this);
    m_modelLog->setHorizontalHeaderItem(0, new QStandardItem(QObject::tr("正常or异常")));
    m_modelLog->setHorizontalHeaderItem(1, new QStandardItem(QObject::tr("事件描述")));
    m_modelLog->setHorizontalHeaderItem(2, new QStandardItem(QObject::tr("事件原因")));
    m_modelLog->setHorizontalHeaderItem(3, new QStandardItem(QObject::tr("发生事件")));
    ui->tableView_Log->setModel(m_modelLog);

    //绑定消息响应
    connect(ui->Button_OpenFilePath, SIGNAL(clicked()), this, SLOT(openFilePath()));//打开文件路径消息响应
    connect(ui->Button_Switch, SIGNAL(clicked()), this, SLOT(displayMsg()));//负载均很器开关消息响应
    connect(ui->Button_Log, SIGNAL(clicked()), this, SLOT(displayLog()));//负载均很器开关消息响应
    connect(&m_udpClientSocket, SIGNAL(recvData()), this, SLOT(addClientMsgToTable()));//接收客户端发送的UDP数据消息
    connect(&m_udpServerSocket, SIGNAL(recvServerData()), this, SLOT(addServerMsgToTable()));//接收服务器发送的UDP数据消息
}

Loder::~Loder()
{
    delete ui;
    if(m_modelMsg != NULL)
    {
        delete m_modelMsg;
        m_modelMsg = NULL;
    }
    if(m_modelServer != NULL)
    {
        delete m_modelServer;
        m_modelServer = NULL;
    }
}

/*
 *信号槽函数，响应打开文件的按钮，初始化配置信息
 */
void Loder::openFilePath()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("open file"), " ",  tr("Allfile(*.*);;txtfile(*.txt)"));
    ui->lineEdit_filePath->setText(fileName);
    configuration(readFile(fileName));

    m_udpClientSocket.initSocket(false);
    m_udpServerSocket.initSocket(true);

    //需要配置好初始化信息后，调试开关才允许使用,否则会因为没有初始化数据导致打开开关出错
    if(!ui->Button_Switch->isEnabled())
        ui->Button_Switch->setEnabled(true);
}

/*
 *addClientMsgtoTable()
 *接收客户端消息函数，将消息记录添加到消息显示面板中，供查看
 */
void Loder::addClientMsgToTable()
{
    ui->tableView_Msg->setFocus();

    QStringList recvMsg = QString(m_udpClientSocket.getRecvMsg()).split("$");
    if(recvMsg.at(recvMsg.length() - 1) == "client")
    {
        fillMsg(recvMsg, false);
        sendMsgToServer(recvMsg);
    }
}

void Loder::addServerMsgToTable()
{
    QStringList recvMsg = QString(m_udpServerSocket.getRecvMsg()).split("$");

    if(recvMsg.at(recvMsg.length() - 1) == "server")
    {
        fillMsg(recvMsg, false);
        sendMsgToClient(recvMsg);
    }
}

/*
 *fillMsg
 *将负载均衡器接收或发送的消息，经过格式转后后，填入消息显示面板
 */
void Loder::fillMsg(QStringList recvMsg, bool RorS)
{
    //对需要进行写的变量加锁，防止同时写变量
    QMutex mutex;
    mutex.lock();

    QString src_id = recvMsg.at(0);
    QString des_id = recvMsg.at(1);
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
    if(RorS)
    {
        msgType += "(send)";
    }
    else
        msgType += "(recieve)";
    QString msgSource = "未知";


    if(recvMsg.at(5) == m_clientPort)
    {
        msgSource = tr("client");

        ++m_clientRecvRightMsg;

        ui->lineEdit_ClientRight->setText(QString("%1").arg(m_clientRecvRightMsg));
        ui->lineEdit_ClientSum->setText(QString("%1").arg(m_clientRecvErrorMsg + m_clientRecvRightMsg));
    }
    else if(recvMsg.at(5) == m_serverPort)
    {
        msgSource = tr("server");
        ++m_serverRecvRightMsg;

        ui->lineEdit_ServerRight->setText(QString("%1").arg(m_serverRecvRightMsg));
        ui->lineEdit_ServerSum->setText(QString("%1").arg(m_serverRecvErrorMsg + m_serverRecvRightMsg));
    }

    QString msgContent = recvMsg.at(4);
    QString msgTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    int rowCount = m_modelMsg->rowCount();
    m_modelMsg->setItem(rowCount, 0, new QStandardItem(msgType));
    m_modelMsg->setItem(rowCount, 1, new QStandardItem(msgSource));
    m_modelMsg->setItem(rowCount, 2, new QStandardItem(src_id));
    m_modelMsg->setItem(rowCount, 3, new QStandardItem(des_id));
    m_modelMsg->setItem(rowCount, 4, new QStandardItem(msgContent));
    m_modelMsg->setItem(rowCount, 5, new QStandardItem(msgTime));

    QStringList logMsg;
    logMsg << tr("normal") << tr("succed in send or recv msg") << tr("send or recv") << msgTime;
    writeLog(logMsg);
    mutex.unlock();
}
/*
 *负载均衡器获取到正确的客户端时间请求信息后，将消息的src_id和des_id修改后发送给服务器端
 *采用循环轮转方法选择服务器的ID号
 *
*/
void Loder::sendMsgToServer(QStringList qClientMsg)
{
    //*采用循环轮转方法选择服务器的ID号，currentServer记录的是当前用到的服务器
    ++currentServer;
    map<QString, QString>::iterator iter = m_server.begin();
    currentServer = currentServer % m_serverNum.toInt();
    for(int i = 0; i < currentServer && iter != m_server.end(); ++i)
    {
        iter++;
    }
    QString serverID = iter->first;
    QStringList qServerMsg;
    qServerMsg << m_loaderID << serverID;
    for(int i = 2; i < qClientMsg.length(); ++i)
    {
        qServerMsg << qClientMsg.at(i);
    }
    cout << endl;
    m_udpServerSocket.sendMsg(qServerMsg, QString::fromStdString(""), m_serverPort.toInt());
    ++m_serverSendMsgSum;
    ui->lineEdit_ServerSendSum->setText(QString("%1").arg(m_serverSendMsgSum));
    qServerMsg << m_clientPort;
    fillMsg(qServerMsg, true);
}

void Loder::sendMsgToClient(QStringList qMsg)
{
    QStringList qServerMsg;
    qServerMsg << m_loaderID << qMsg.at(1) << qMsg.at(2) << qMsg.at(3) << qMsg.at(4);
    m_udpClientSocket.sendMsg(qServerMsg, QString::fromStdString(""), m_clientPort.toInt());
    ++m_clentSendMsgSum;
    ui->lineEdit_ClientSendSum->setText(QString("%1").arg(m_clentSendMsgSum));
    qServerMsg << m_serverPort;
    fillMsg(qServerMsg, true);
}

/*
 *开关负载均衡器，通过响应button按钮实现
 */
void Loder::displayMsg()
{
    if(ui->Button_Switch->text() == tr("调试(close)"))
    {
        m_udpClientSocket.connectOther(m_clientPort.toInt());
        m_udpServerSocket.connectOther(m_serverPort.toInt());
        ui->tabWidget->setCurrentIndex(1);
        ui->Button_Switch->setText(tr("调试(open)"));
    }
    else if(ui->Button_Switch->text() == tr("调试(open)"))
    {
        m_udpClientSocket.closeSocket();
        m_udpServerSocket.closeSocket();
        ui->tabWidget->setCurrentIndex(0);
        ui->Button_Switch->setText(tr("调试(close)"));
    }
}

/*
 *readFile
 *负载均衡器读取配置文件
 */
QStringList Loder::readFile(QString fileName)
{
    QStringList fileContent;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return fileContent;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        fileContent << line;
    }
    file.close();
    return fileContent;
}

/*
 *configuration
 *通过读取的配置文件，将信息填入面板中，供使用者查看
 */
void Loder::configuration(QStringList list)
{
    ui->lineEdit_ID->setText(list.at(0).split(" ").at(1));
    ui->lineEdit_clientPort->setText(list.at(1).split(" ").at(1));
    ui->lineEdit_serverPort->setText(list.at(2).split(" ").at(1));
    ui->lineEdit_ServerNum->setText(list.at(3).split(" ").at(1));
    m_loaderID = ui->lineEdit_ID->text();
    m_serverNum = ui->lineEdit_ServerNum->text();//;
    m_serverPort = ui->lineEdit_serverPort->text();
    m_clientPort = ui->lineEdit_clientPort->text();

    for(int i = 5; i < list.length(); ++i)
    {
        QString id = list.at(i).split(" ").at(0);
        QString udpPort = list.at(i).split(" ").at(1);
        m_server.insert(pair<QString, QString>(id, udpPort));
        int rowCount = m_modelServer->rowCount();
        m_modelServer->setItem(rowCount, 0, new QStandardItem(id));
        m_modelServer->setItem(rowCount, 1, new QStandardItem(udpPort));
    }
}
/*
 *writeLog
 *填写日志
 */
void Loder::writeLog(QStringList msg)
{
    QString fileName = "log.txt";
    QFile file(fileName);
    if (!file.open(QIODevice::Append))
        return;

    QTextStream out(&file);
    for(int i = 0; i < msg.length() - 1; ++i)
    {
        out << msg.at(i) << "#";
    }
    out << msg.at(msg.length() - 1);
    out << "\r\n";
    file.close();
}

void Loder::displayLog()
{
    ui->tabWidget->setCurrentIndex(2);
    QString fileName = "log.txt";
    QStringList logContent = readFile(fileName);
    for(int i = 0; i < logContent.length(); ++i)
    {
        QStringList lineContent = logContent.at(i).split("#");
        int rowCount = i;
        for(int j = 0; j < lineContent.length(); ++j)
        {
            m_modelLog->setItem(rowCount, j, new QStandardItem(lineContent.at(j)));
        }
    }
}
