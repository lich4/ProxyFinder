#include "utils.h"

int GetHttpRequest(const QString& url, QString& response, const QString& method, const QString& postdata,
                   const QNetworkProxy& proxy, int& timeout) {
    QNetworkAccessManager manager;
    QNetworkReply* reply;
    QNetworkRequest request;
    QTimer timer;
    QTime timetester;

    timer.setInterval(timeout);
    timer.setSingleShot(true);

    manager.setProxy(proxy);

    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/x-www-form-urlencoded"));
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QVariant("Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; rv:11.0) like Gecko"));
    if (method == "get") {
        reply = manager.get(request);
    }
    else if (method == "post") {
        reply = manager.post(request, postdata.toUtf8());
    }
    else {
        return 10001; // Param Error
    }
    QEventLoop loop;
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start();
    timetester.start();
    loop.exec();  // 启动事件循环
    timeout = timetester.elapsed();
    int err = reply->error();
    int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (timer.isActive()) {  // 处理响应
        timer.stop();
        qDebug() << "net err:" << err ;
        if (err != QNetworkReply::NoError) {
            timeout = 100000;
        } else {
            QByteArray bytes = reply->readAll();
            response = QString::fromUtf8(bytes.data(), bytes.size());
        }
    } else {  // 处理超时
        QObject::disconnect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        reply->abort();
    }
    loop.deleteLater();
    reply->deleteLater();
    log(QString("GetHttpRequest %1 proxyhost=%2 proxyport=%3 err=%4 code=%5 timeout=%6 datalen=%7")
        .arg(url).arg(proxy.hostName()).arg(proxy.port()).arg(err).arg(code).arg(timeout).arg(response.length()));
    return err;
}

int GetHttpRequestNoProxy(const QString& url, QString& response, int timeout, const QString& method,
                               const QString& postdata) {
    QNetworkProxy proxy(QNetworkProxy::NoProxy);
    //QNetworkProxy proxy(QNetworkProxy::HttpProxy, "127.0.0.1", 8888);
    return GetHttpRequest(url, response, method, postdata, proxy, timeout);
}

QString base64_decode(const QString& text) {
    return QString(QByteArray::fromBase64(text.toLatin1()));
}

QString base64_encode(const QString& text) {
    return QString(text.toLatin1().toBase64());
}

QString tidy_html(const QString& content) {
    QString result;
    TidyBuffer output = { nullptr, nullptr, 0, 0, 0 }, errbuf = { nullptr, nullptr, 0, 0, 0 };
    TidyDoc doc = tidyCreate();
    tidyOptSetBool(doc, TidyXhtmlOut, yes);
    tidySetErrorBuffer(doc, &errbuf);
    tidyParseString(doc, content.toLatin1().data());
    tidyCleanAndRepair(doc);
    tidyRunDiagnostics(doc);
    tidyOptSetBool(doc, TidyForceOutput, yes);
    tidySaveBuffer(doc, &output);
    result = reinterpret_cast<const char*>(output.bp);
    tidyBufFree( &output );
    tidyBufFree( &errbuf );
    tidyRelease( doc );
    return result;
}

bool is_server_port_open (const QString& hostname, quint16 port, int timeout) {
    const int con_wait_time = timeout; // ms
    {
        QUdpSocket socket;
        socket.abort();
        socket.connectToHost(hostname, port);
        if (socket.waitForConnected(con_wait_time)) {
            socket.close();
            return true;
        }
    } {
        QTcpSocket socket;
        socket.abort();
        socket.connectToHost(hostname, port);
        if (socket.waitForConnected(con_wait_time)) {
            socket.close();
            return true;
        }
    }
    return false;
}

static QMutex fmutex;
void log(const QString& content) {
    fmutex.lock();
    QFile file("log.log");
    if(!file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        return;
    }
    QTextStream in(&file);
    in << content << "\r\n";
    file.close();
    fmutex.unlock();
}

void printxml(pugi::xml_node node, int depth) {
    QString prefix, attribs;
    for (int i = 0; i < depth; i++) {
        prefix += "  ";
    }
    for (pugi::xml_attribute attrib : node.attributes()) {
        attribs +=  QString("%1=%2 ").arg(attrib.name()).arg(attrib.value());
    }
    qDebug() << prefix + "<" + node.name() + ":" + attribs + " =>" + node.text().as_string() + ">";
    for (pugi::xml_node child : node.children()) {
        printxml(child, depth + 1);
    }
    qDebug() << prefix + "</" + node.name() + ">";
}
