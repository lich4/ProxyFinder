#ifndef COMMON_H
#define COMMON_H

// C++ Headers
#include <algorithm>
#include <iostream>
#include <tr1/functional>

// Qt Headers
#include <QAbstractXmlNodeModel>
#include <QDebug>
#include <QEventLoop>
#include <QFont>
#include <QFuture>
#include <QGuiApplication>
#include <QIcon>
#include <QListView>
#include <QMap>
#include <QMessageBox>
#include <QMutex>
#include <QObject>
#include <QProcess>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickView>
#include <QSettings>
#include <QString>
#include <QtConcurrent>
#include <QtAlgorithms>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtGui/QRegExpValidator>
#include <QThread>
#include <QTimer>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtNetwork/QDnsLookup>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QUdpSocket>
#include <QtWidgets/QApplication>
#include <QtXmlPatterns/QXmlQuery>
#include <QtXmlPatterns/QXmlResultItems>
#include <QUrl>
#include <QVariant>
#include <QVector>

#include "pugixml.hpp"
#include "pugiconfig.hpp"

class FunctionTransfer : public QObject {
    Q_OBJECT
public:
    ///@brief 构造函数
    explicit FunctionTransfer(QObject *parent = 0);
public:
    ///@brief 制定函数f在main中执行
    static void execinmain(std::tr1::function<void()> f);
signals:
    ///@brief 在别的线程有函数对象传来
    void comming(std::tr1::function<void()> f);
public slots:
    ///@brief 执行函数对象
    void exec(std::tr1::function<void()> f);
};



#endif // COMMON_H
