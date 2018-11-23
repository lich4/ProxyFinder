#include "httpserver.h"

HttpServer& HttpServer::getinst() {
    static HttpServer inst(qApp);
    return inst;
}

HttpServer::HttpServer(QObject* parent) : QObject(parent) {
    m_tcpServer = 0;
    tcpSocket = 0;
}

HttpServer& HttpServer::init(QString data, quint16 port) {
    this->data = data;
    this->port = port;
    return *this;
}

bool HttpServer::start()
{
    if (m_tcpServer != 0) {
        return true;
    }
    m_tcpServer = new QTcpServer();
    if (!m_tcpServer->listen(QHostAddress::Any, port)) {
        delete m_tcpServer;
        m_tcpServer = 0;
        qDebug() << "listening failed";
        return false;
    }
    m_tcpServer->setMaxPendingConnections(1); //设置最大允许连接数
    connect(m_tcpServer, SIGNAL(newConnection()), this, SLOT(newConnectSlot()));
    return true;
}

void HttpServer::stop() {
    if (tcpSocket != 0) {
        tcpSocket->close();
        tcpSocket = 0;
    }
    if (m_tcpServer != 0) {
        m_tcpServer->close();
        delete m_tcpServer;
        m_tcpServer = 0;
    }
}

void HttpServer::newConnectSlot()
{
    tcpSocket = m_tcpServer->nextPendingConnection();
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(getMessage()));
}

void HttpServer::getMessage()
{
    QString http = "HTTP/1.1 200 OK\r\n";
    http += "Content-Type: text/html;charset=utf-8\r\n";
    http += QString("Content-Length: %1\r\n\r\n").arg(this->data.length());

    if(tcpSocket != 0) {
        QByteArray headData, data;
        headData.append(http);
        tcpSocket->write(headData);
        tcpSocket->write(this->data.toLatin1());
    }
}

