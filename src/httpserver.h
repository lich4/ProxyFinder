#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "common.h"

class HttpServer : public QObject {
    Q_OBJECT
public:
    static HttpServer& getinst();
    HttpServer(QObject* parent);
    HttpServer& init(QString data, quint16 port);
    bool start();
    void stop();
public slots:
    void newConnectSlot();
    void getMessage();
private:
    QTcpServer* m_tcpServer;
    QTcpSocket* tcpSocket;
    QString data;
    quint16 port;
};

#endif // HTTPSERVER_H
