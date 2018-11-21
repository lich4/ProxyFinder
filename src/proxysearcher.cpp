#include "proxysearcher.h"

static FunctionTransfer main_thread_forward;
void FunctionTransfer::execinmain(std::tr1::function<void()> f) {
    main_thread_forward.exec(f);
}

FunctionTransfer::FunctionTransfer(QObject *parent) : QObject(parent) {
    connect(this, SIGNAL(comming(std::tr1::function<void()>)), this,
            SLOT(exec(std::tr1::function<void()>)), Qt::BlockingQueuedConnection);
}

void FunctionTransfer::exec(std::tr1::function<void()> f) {
    if(QThread::currentThread() == QApplication::instance()->thread()) {
        f();
    }
    else {
        emit this->comming(f);
    }
}

int ProxySearch:: test_proxy_server_list() {
    mlock.lock();
    proxy_list.enqueue(QNetworkProxy());
    mlock.unlock();
    while (!is_search_done || !proxy_list.isEmpty()) {
        if (!proxy_list.isEmpty()) {
            mlock.lock();
            QNetworkProxy proxy = proxy_list.dequeue();
            mlock.unlock();
            for (const QString& ip : ip_list) {
                if (is_cancel) {
                    break;
                }
                ProxyValidateThread* th = new ProxyValidateThread(this, proxy, ip);
                validate_threadpool.start(th);
            }
        }
        QThread::msleep(100);
        if (is_cancel) {
            break;
        }
    }
    return 0;
}

int ProxySearch::get_proxy_server_list() {
    proxy_list.clear();
    proxy_cache.clear();
    max_runlist.clear();
    is_search_done = false;
    is_searching = true;
    is_validating = true;
    is_validate_done = false;
    WorkThread test_thread(this);
    test_thread.start();

    search_threadpool.clear();
    validate_threadpool.clear();
    for (const QString& url : search_src.split(";")) {
        if (is_cancel) {
            break;
        }
        QJsonDocument doc;
        if (url.startsWith("http://") || url.startsWith("https://") ||
                url.startsWith("ftp://")) {
            QString response;
            int status = GetHttpRequestNoProxy(url, response, search_tmout);
            if (status != 0 || response.isEmpty()) {
                return -1;
            }
            int begin = response.indexOf("json::");
            int end = response.indexOf("::json");
            if (begin != -1) {
                QString encode_data = response.mid(begin + 6, end - begin - 6);
                QString decode_data = base64_decode(encode_data);
                QJsonParseError error;
                doc = QJsonDocument::fromJson(decode_data.toLatin1(), &error);
            } else {
                QJsonParseError error;
                doc = QJsonDocument::fromJson(response.toLatin1(), &error);
            }
        } else {
            QFile file(url);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QByteArray allData = file.readAll();
                QJsonParseError error;
                doc = QJsonDocument::fromJson(allData, &error);
                file.close();
            }
        }
        if (doc.isNull()) {
            return -2;
        }
        QJsonObject object = doc.object();
        QJsonArray jserver = object.value("proxy_server").toArray();
        for (QJsonValue val : jserver) {
            QJsonObject obj = val.toObject();
            QString type = obj.value("type").toString();
            QString baseurl = obj.value("url").toString();
            if (type == "txt") {
                if (is_cancel) {
                    break;
                }
                ProxySearchThread* th = new ProxySearchThread(this, baseurl, "", "", "", type);
                search_threadpool.start(th);
            } else if (type == "html") {
                QString root = obj.value("root").toString();
                QString host = obj.value("host").toString();
                QString port = obj.value("port").toString();
                int beginpage = 1;
                int endpage = 1;
                QString pagequery = "";
                if (obj.contains("page")) {
                    QString page = obj.value("page").toString();
                    QStringList list = page.split(":");
                    pagequery = list[0];
                    beginpage = list[1].toInt();
                    endpage = list[2].toInt();
                }
                for (int i = beginpage; i <= endpage; i++) {
                    if (endpage == beginpage) {
                        if (is_cancel) {
                            break;
                        }
                        ProxySearchThread* th = new ProxySearchThread(
                                    this, baseurl, root, host, port, type);
                        search_threadpool.start(th);
                    } else {
                        if (is_cancel) {
                            break;
                        }
                        ProxySearchThread* th = new ProxySearchThread(
                                    this, baseurl + pagequery.arg(i), root, host, port, type);
                        search_threadpool.start(th);
                    }
                }
            }
        }
    }
    search_threadpool.waitForDone();
    is_search_done = true;
    is_searching = false;
    //update_status("搜索代理:结束 验证代理:运行");

    test_thread.wait();
    validate_threadpool.waitForDone();
    is_validating = false;
    is_validate_done = true;
    //update_status("搜索代理:结束 验证代理:结束");
    return 0;
}

void ProxyValidateThread::run() {
    QString response;
    QString result = "无效";
    int timeout = proxycls->validate_tmout; // ms
    int totaltime = 0;
    for (int i = 0; i < proxycls->validate_count; i++) {
        if (proxycls->is_cancel) {
            return;
        }
        int status = GetHttpRequest(ip, response, "get", "", proxy, timeout);
        if (status == QNetworkReply::NoError) {
            totaltime += timeout;
        } else {
            totaltime += 10000;
        }
    }
    int tt = totaltime / proxycls->validate_count;// do work
    // 保留topN
    if (response.indexOf(proxycls->validate_key) != -1) {
        proxycls->mlock.lock();
        log(QString("ProxyValid ip=%1 host=%2 port=%3 timeout=%4 valid")
            .arg(ip).arg(hostname).arg(port).arg(tt));
        result = "有效";
        proxycls->max_runlist.push_back(ProxyListItem(hostname, port, tt));
        if (proxycls->max_runlist.length() > ProxySearch::MAX_LNE_NUM) {
            qSort(proxycls->max_runlist.begin(), proxycls->max_runlist.end(), ProxyListItem::comp);
            proxycls->max_runlist.pop_back();
        }
        proxycls->mlock.unlock();
        // 更新视图
        proxycls->update_runlist();
    } else {
        log(QString("ProxyValid ip=%1 host=%2 port=%3 timeout=%4 invalid")
            .arg(ip).arg(hostname).arg(port).arg(timeout));
    }
}

void ProxySearchThread::run() {
    QString content;
    QString _host = QUrl(url).host();
    if (proxycls->is_cancel) {
        return;
    }
    int result_count = 0;
    if (type == "txt") {
        GetHttpRequestNoProxy(url, content, proxycls->search_tmout);
        QRegExp reg("(\\S+):([0-9]{1,5})");
        QStringList list = content.split("\n");
        for (QString& s : list) { // 逐行匹配
            if (proxycls->is_cancel) {
                return;
            }
            if (reg.indexIn(s) > -1) {
                QString thost = reg.cap(1);
                quint16 tport = static_cast<quint16>(reg.cap(2).toInt());
                QString tag = QString("%1:%2").arg(thost).arg(tport);
                proxycls->mlock.lock();
                log(QString("ProxySearch url=%1 host=%2 port=%3").arg(url).arg(thost).arg(tport));
                if (!proxycls->proxy_cache.contains(tag)) {
                    proxycls->proxy_list.enqueue(QNetworkProxy(QNetworkProxy::HttpProxy, thost, tport));
                    proxycls->proxy_cache.insert(tag);
                    result_count++;
                }
                proxycls->mlock.unlock();
            }
        }
    } else if (type == "html") {
        GetHttpRequestNoProxy(url, content, proxycls->search_tmout);
        content = tidy_html(content);
        pugi::xml_document doc;
        doc.load_string(content.toLatin1().data());
        pugi::xpath_node_set xpathnodes = doc.select_nodes(root.toLatin1().data());
        for (const pugi::xpath_node& xpathnode : xpathnodes) {
            if (proxycls->is_cancel) {
                return;
            }
            const pugi::xml_node& xmlnode = xpathnode.node();
            pugi::xpath_node hostnode = xmlnode.select_node(host.toLatin1().data());
            pugi::xpath_node portnode = xmlnode.select_node(port.toLatin1().data());
            QString thost = hostnode.node().value();
            quint16 tport = static_cast<quint16>(QString(portnode.node().value()).toInt());
            if (tport == 0) {
                continue;
            }
            QString tag = QString("%1:%2").arg(thost).arg(tport);
            proxycls->mlock.lock();
            log(QString("ProxySearch url=%1 host=%2 port=%3").arg(url).arg(thost).arg(tport));
            if (!proxycls->proxy_cache.contains(tag)) {
                proxycls->proxy_list.enqueue(QNetworkProxy(QNetworkProxy::HttpProxy, thost, tport));
                proxycls->proxy_cache.insert(tag);
                result_count++;
            }
            result_count++;
            proxycls->mlock.unlock();
        }
    }
}
