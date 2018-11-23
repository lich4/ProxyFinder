#ifndef PROXYSEARCHER_H
#define PROXYSEARCHER_H

#include "common.h"
#include "utils.h"
#include "httpserver.h"

#include "libxml/tree.h"
#include "libxml/parser.h"
#include "libxml/xpath.h"
#include "libtidy/tidy.h"
#include "libtidy/tidybuffio.h"
#include "libtidy/tidyenum.h"
#include "libtidy/tidyplatform.h"

class ProxyListItem {
public:
    QString hostname;
    quint16 port;
    int timeout;
public:
    ProxyListItem() : timeout(1000000) { }
    ProxyListItem(QString& hostname, quint16 port, int timeout) {
        this->hostname = hostname;
        this->port = port;
        this->timeout = timeout;
    }
    ProxyListItem(const ProxyListItem& newobj) {
        hostname = newobj.hostname;
        port = newobj.port;
        timeout = newobj.timeout;
    }
    static bool comp(ProxyListItem& itema, ProxyListItem& itemb) {
        return itema.timeout < itemb.timeout;
    }
};

class ProxySearch : public QObject {
    friend class ProxyValidateThread;
    friend class ProxySearchThread;
    Q_OBJECT
public:
    enum {
        MAX_COL_NUM = 3,
        MAX_LNE_NUM = 8,
    };
    virtual ~ProxySearch() {
        cancel_search_proxy();
    }
    static ProxySearch* getinst(QQmlApplicationEngine* engine = nullptr) {
        static ProxySearch inst;
        if (engine != nullptr) {
            inst.engine = engine;
            // 设置默认值
            inst.search_src = "proxy"; // https://www.jianshu.com/p/b6958c519442
            inst.dns_src = "https://public-dns.info/nameservers.txt";
            inst.validate_src = "http://www.guimp.com";
            inst.validate_key = "html";
            inst.search_tmout = 5000;
            inst.validate_tmout = 5000;
            inst.validate_count = 3;
            inst.thread_count = 60;
            inst.fulldns = false; // 默认不开启全DNS搜索
            inst.is_search_done = true;
            inst.is_cancel = true;
            inst.is_validating = false;
            inst.is_validate_done = false;
            inst.root_proxyfinder = engine->rootObjects().first();
            inst.lastcheck_dns = -1;
            inst.lastcheck_proxy = -1;
            inst.httpserv_port = 81;
            inst.validate_threadpool.setMaxThreadCount(inst.thread_count);
            inst.search_threadpool.setMaxThreadCount(1);
        }
        return &inst;
    }
    int stop_test_proxy() {
        for(auto future : futures) {
            future.cancel();
        }
        futures.clear();
        return 0;
    }
    void update_runlist() {
        mlock.lock();
        FunctionTransfer::execinmain([&](){
            QVariant retval = 0;
            QVariant arg0 = -1;
            QVariant arg1;
            QMetaObject::invokeMethod(root_https_search, "clear", Qt::DirectConnection,
                Q_RETURN_ARG(QVariant, retval));
            for (int i = 0; i < max_runlist.size(); i++) {
                ProxyListItem& it = max_runlist[i];
                arg1 = QString("%1:%2 => %3 ms").arg(it.hostname).arg(it.port).arg(it.timeout);
                QMetaObject::invokeMethod(root_https_search, "update", Qt::DirectConnection,
                    Q_RETURN_ARG(QVariant, retval), Q_ARG(QVariant, arg0), Q_ARG(QVariant, arg1));
            }
        });
        QThread::msleep(100);
        mlock.unlock();
    }
    Q_INVOKABLE void do_dns_lookup() {
        if (is_searching) {
            return;
        }
        QtConcurrent::run([&]() {
            // 激活进度条
            FunctionTransfer::execinmain([&](){
                QVariant retval = 0;
                QMetaObject::invokeMethod(root_dns_resolve, "start", Qt::DirectConnection,
                    Q_RETURN_ARG(QVariant, retval));
            });
            is_searching = true;
            is_search_done = false;
            is_cancel = false;
            QString validate_host = QUrl(validate_src).host();
            int p = validate_src.indexOf(validate_host);
            QString pre = validate_src.mid(0, p);
            QString post = validate_src.mid(p + validate_host.length());
            QSet<QString> ip_set;
            ip_list.clear();
            QString res;
            GetHttpRequestNoProxy(dns_src, res, 10000);
            QList<QFuture<void> > futures;
            auto lambda = [&] (QString host) {
                QDnsLookup dns;
                QEventLoop loop;
                if (is_cancel) {
                    return;
                }
                QObject::connect(&dns, &QDnsLookup::finished, &loop, &QEventLoop::quit);
                dns.setType(QDnsLookup::A);
                dns.setNameserver(QHostAddress(host));
                dns.setName(validate_host);
                dns.lookup();
                loop.exec();
                if (dns.error() != QDnsLookup::NoError) {
                    return;
                }
                foreach (const QDnsHostAddressRecord &record, dns.hostAddressRecords()) {
                    QString ip = record.value().toString();
                    mlock.lock();
                    if (!ip_set.contains(ip)) {
                        FunctionTransfer::execinmain([&](){
                            QVariant retval = 0;
                            QVariant arg0 = -1;
                            QVariant arg1 = ip;
                            QMetaObject::invokeMethod(root_dns_resolve, "update", Qt::DirectConnection,
                                Q_RETURN_ARG(QVariant, retval), Q_ARG(QVariant, arg0), Q_ARG(QVariant, arg1));
                        });
                        ip_set.insert(ip);
                        ip_list.push_back(ip);
                    }
                    QThread::msleep(100);
                    mlock.unlock();
                }
            };
            QStringList lines = res.split("\n");
            for (QString& line : lines) {
                futures.append(QtConcurrent::run(lambda, line));
            }
            for(auto future : futures) {
                if (is_cancel) {
                    break;
                }
                future.waitForFinished();
            }
            is_search_done = true;
            is_cancel = true;
            is_searching = false;
            // 禁止进度条
            FunctionTransfer::execinmain([&](){
                QVariant retval = 0;
                QMetaObject::invokeMethod(root_dns_resolve, "stop", Qt::DirectConnection,
                    Q_RETURN_ARG(QVariant, retval));
            });
        });
    }
    Q_INVOKABLE bool get_dns_resolve_progress_visible() {
        return is_searching;
    }
    Q_INVOKABLE void get_dns_resolve_lock() {
        mlock.lock();
    }
    Q_INVOKABLE void get_dns_resolve_unlock() {
        mlock.unlock();
    }
    Q_INVOKABLE int get_dns_resolve_listview_size() {
        return ip_list.size();
    }
    Q_INVOKABLE QString get_dns_resolve_listview_item(int i) {
        return ip_list[i];
    }
    Q_INVOKABLE void cancel_dns_lookup() {
        if (!is_searching) {
            return;
        }
        if (is_cancel) {
            return;
        }
        is_cancel = true;
        if (!is_search_done) {
            QThread::sleep(1);
        }
        is_searching = false;
        FunctionTransfer::execinmain([&](){
            QVariant retval = 0;
            QMetaObject::invokeMethod(root_dns_resolve, "stop", Qt::DirectConnection,
                Q_RETURN_ARG(QVariant, retval));
        });
    }
    Q_INVOKABLE void register_dns_resolve(QObject* root) {
        root_dns_resolve = root;
    }
    Q_INVOKABLE void register_https_search(QObject* root) {
        root_https_search = root;
    }
    int get_proxy_server_list();
    int test_proxy_server_list();
    Q_INVOKABLE void set_search_source(const QString& s) {
        search_src = s;
    }
    Q_INVOKABLE QString get_search_source() {
        return search_src;
    }
    Q_INVOKABLE void set_dns_source(const QString& s) {
        dns_src = s;
    }
    Q_INVOKABLE QString get_dns_source() {
        return dns_src;
    }
    Q_INVOKABLE void set_validate_source(const QString& s) {
        validate_src = s;
    }
    Q_INVOKABLE QString get_validate_source() {
        return validate_src;
    }
    Q_INVOKABLE void set_validate_key(const QString& s) {
        validate_key = s;
    }
    Q_INVOKABLE QString get_validate_key() {
        return validate_key;
    }
    Q_INVOKABLE void set_search_timeout(int t) {
        search_tmout = t;
    }
    Q_INVOKABLE int get_search_timeout() {
        return search_tmout;
    }
    Q_INVOKABLE void set_validate_timeout(int t) {
        validate_tmout = t;
    }
    Q_INVOKABLE int get_validate_timeout() {
        return validate_tmout;
    }
    Q_INVOKABLE void set_validate_count(int c) {
        validate_count = c;
    }
    Q_INVOKABLE int get_validate_count() {
        return validate_count;
    }
    Q_INVOKABLE void set_thread_count(int c) {
        thread_count = c;
        validate_threadpool.setMaxThreadCount(thread_count);
    }
    Q_INVOKABLE int get_thread_count() {
        return thread_count;
    }
    Q_INVOKABLE void set_fulldns(bool checked) {
        fulldns = checked;
    }
    Q_INVOKABLE void set_httpserv_port(QString port) {
        httpserv_port = port.toInt();
    }
    Q_INVOKABLE QString get_httpserv_port() {
        return QString("%1").arg(httpserv_port);
    }
    Q_INVOKABLE void do_search_proxy() {
        if (is_searching || is_validating) {
            return;
        }
        is_cancel = false;
        ip_list.clear();
        QtConcurrent::run([&]() {
            FunctionTransfer::execinmain([&](){
                QVariant retval = 0;
                QMetaObject::invokeMethod(root_https_search, "start", Qt::DirectConnection,
                    Q_RETURN_ARG(QVariant, retval));
            });
            // 从hosname获取ip列表
            ip_list.push_back(validate_src);
            // 从代理源获取代理列表
            get_proxy_server_list();
            FunctionTransfer::execinmain([&](){
                QVariant retval = 0;
                QMetaObject::invokeMethod(root_https_search, "stop", Qt::DirectConnection,
                    Q_RETURN_ARG(QVariant, retval));
            });
        });
    }
    Q_INVOKABLE void cancel_search_proxy() {
        if (!is_searching && !is_validating) {
            return;
        }
        if (is_cancel) {
            return;
        }
        is_cancel = true;
        if (!is_search_done || !is_validate_done) {
            QThread::sleep(1);
        }
        is_searching = false;
        is_validating = false;
        FunctionTransfer::execinmain([&](){
            QVariant retval = 0;
            QMetaObject::invokeMethod(root_https_search, "stop", Qt::DirectConnection,
                Q_RETURN_ARG(QVariant, retval));
        });
    }
    Q_INVOKABLE bool get_https_search_progress_visible() {
        return is_searching;
    }
    Q_INVOKABLE void get_https_search_lock() {
        mlock.lock();
    }
    Q_INVOKABLE void get_https_search_unlock() {
        mlock.unlock();
    }
    Q_INVOKABLE int get_https_search_listview_size() {
        return max_runlist.size();
    }
    Q_INVOKABLE QString get_https_search_listview_item(int i) {
        ProxyListItem& it = max_runlist[i];
        return QString("%1:%2 => %3 ms").arg(it.hostname).arg(it.port).arg(it.timeout);
    }
    Q_INVOKABLE void set_https_search_lastcheck(int last) {
        lastcheck_proxy = last;
    }
    Q_INVOKABLE int get_https_search_lastcheck() {
        return lastcheck_proxy;
    }
    Q_INVOKABLE void do_cancel_proxy() {
        stop_test_proxy();
        // 取消设置IE代理
        QSettings proxyenableValue(
                    "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
                    QSettings::NativeFormat);
        proxyenableValue.setValue("ProxyEnable", 0);
    }
    Q_INVOKABLE void do_apply_proxy(int index) {
        // 设置IE代理
        QSettings proxyenableValue(
                    "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
                    QSettings::NativeFormat);
        if (max_runlist.length() > index) {
            QString hostname = max_runlist[index].hostname;
            quint16 port = max_runlist[index].port;
            proxyenableValue.setValue("ProxyEnable", 1);
            proxyenableValue.setValue("ProxyServer", QString("%1:%2").arg(hostname).arg(port));
        }
    }
    Q_INVOKABLE void register_ssr_search(QObject* root) {
        root_ssr_search = root;
    }
Q_INVOKABLE bool get_ssr_search_progress_visible() {
        return is_searching;
    }
    Q_INVOKABLE void get_ssr_search_lock() {
        mlock.lock();
    }
    Q_INVOKABLE void get_ssr_search_unlock() {
        mlock.unlock();
    }
    Q_INVOKABLE int get_ssr_search_listview_size() {
        return ssr_list.size();
    }
    Q_INVOKABLE QString get_ssr_search_listview_item(int i) {
        return ssr_list[i];
    }
    void get_ssr_server_list() {
        QSet<QString> candilist;
        QString url, root, content, decoded;
        pugi::xml_document doc;
        pugi::xpath_node_set xpathnodes;
        int timeout = this->search_tmout;
        content = "";
        GetHttpRequestNoProxy("https://prom-php.herokuapp.com/cloudfra_ssr.txt", content, timeout);
        decoded = base64_decode(content);
        if (decoded.length()) {
            if (decoded.contains('\r')) {
                for (const QString& line : decoded.split("\r\n")) {
                    candilist.insert(line);
                }
            } else if (decoded.contains('\n')) {
                for (const QString& line : decoded.split("\n")) {
                    candilist.insert(line);
                }
            } else {
                candilist.insert(decoded);
            }
        }
        content = "";
        GetHttpRequestNoProxy("https://raw.githubusercontent.com/ImLaoD/sub/master/ssrshare.com", content, timeout);
        decoded = base64_decode(content);
        if (decoded.length()) {
            if (decoded.contains('\r')) {
                for (const QString& line : decoded.split("\r\n")) {
                    candilist.insert(line);
                }
            } else if (decoded.contains('\n')) {
                for (const QString& line : decoded.split("\n")) {
                    candilist.insert(line);
                }
            } else {
                candilist.insert(decoded);
            }
        }
        content = "";
        GetHttpRequestNoProxy("https://raw.githubusercontent.com/forpw2009/lpss2009/master/v2ray/ssrr", content, timeout);
        decoded = base64_decode(content);
        if (decoded.length()) {
            if (decoded.contains('\r')) {
                for (const QString& line : decoded.split("\r\n")) {
                    candilist.insert(line);
                }
            } else if (decoded.contains('\n')) {
                for (const QString& line : decoded.split("\n")) {
                    candilist.insert(line);
                }
            } else {
                candilist.insert(decoded);
            }
        }
        content = "";
        GetHttpRequestNoProxy("https://yzzz.ml/freessr/", content, timeout);
        decoded = base64_decode(content);
        if (decoded.length()) {
            if (decoded.contains('\r')) {
                for (const QString& line : decoded.split("\r\n")) {
                    candilist.insert(line);
                }
            } else if (decoded.contains('\n')) {
                for (const QString& line : decoded.split("\n")) {
                    candilist.insert(line);
                }
            } else {
                candilist.insert(decoded);
            }
        }
        content = "";
        GetHttpRequestNoProxy("https://gdmi.weebly.com/3118523398online.html", content, timeout);
        doc.load_string(tidy_html(content).toLatin1().data());
        xpathnodes = doc.select_nodes("//tr[@class='wsite-multicol-tr']//a//@href");
        for (const pugi::xpath_node& xpathnode : xpathnodes) {
            QString ssrserv = xpathnode.attribute().value();
            candilist.insert(ssrserv);
        }
        content = "";
        GetHttpRequestNoProxy("https://x.ishadowx.net/", content, timeout);
        doc.load_string(tidy_html(content).toLatin1().data());
        xpathnodes = doc.select_nodes("//div[@class='hover-text']");
        for (const pugi::xpath_node& xpathnode : xpathnodes) {
            QString ip, port, pass, meth, proto, obscure;
            for (const pugi::xpath_node child : xpathnode.node().children()) {
                QString text = child.node().text().as_string();
                if (text.startsWith("IP")) {
                    ip = child.node().select_node("span/text()").node().value();
                } else if (text.startsWith("Port")) {
                    port = child.node().select_node("span/text()").node().value();
                } else if (text.startsWith("Password")) {
                    pass = child.node().select_node("span/text()").node().value();
                } else if (text.startsWith("Method")) {
                    meth = text.replace("Method:", "");
                } else if (text.startsWith("auth")) { // ssr
                    proto = text;
                    if (text.contains(' ')) {
                        proto = text.split(' ')[0];
                        obscure = text.split(' ')[1];
                    }
                }
            }
            if (ip == "") {
                continue;
            }
            if (proto == "") {
                QString origin = QString("%1:%2:%3:%4:%5:%6/?obfsparam=").arg(ip).arg(port).arg("origin")
                        .arg(meth).arg("plain").arg(base64_encode(pass));
                candilist.insert("ssr://" + base64_encode(origin));
            } else {
                QString origin = QString("%1:%2:%3:%4:%5:%6/?obfsparam=").arg(ip).arg(port).arg(proto)
                        .arg(meth).arg(obscure).arg(base64_encode(pass));
                candilist.insert("ssr://" + base64_encode(origin));
            }
        }
        mlock.lock();
        for (const QString& ssr : candilist) {
            ssr_list.push_back(ssr);
            FunctionTransfer::execinmain([&](){
                QVariant retval = 0;
                QVariant arg0 = -1;
                QVariant arg1 = ssr;
                QMetaObject::invokeMethod(root_ssr_search, "update", Qt::DirectConnection,
                    Q_RETURN_ARG(QVariant, retval), Q_ARG(QVariant, arg0), Q_ARG(QVariant, arg1));
            });
            QThread::msleep(200);
        }
        mlock.unlock();
    }
    Q_INVOKABLE void do_search_ssr() {
        if (is_searching) {
            return;
        }
        is_cancel = false;
        ssr_list.clear();
        QtConcurrent::run([&]() {
            FunctionTransfer::execinmain([&](){
                QVariant retval = 0;
                QMetaObject::invokeMethod(root_ssr_search, "start", Qt::DirectConnection,
                    Q_RETURN_ARG(QVariant, retval));
            });
            is_searching = true;
            is_search_done = false;
            // 搜索ssr服务器
            get_ssr_server_list();
            QString data = QString(base64_encode(ssr_list.join("\n")));
            is_searching = false;
            is_search_done = true;
            FunctionTransfer::execinmain([&](){
                // 建立http服务器
                HttpServer::getinst().init(data, (quint16)httpserv_port).start();
                QVariant retval = 0;
                QMetaObject::invokeMethod(root_ssr_search, "stop", Qt::DirectConnection,
                    Q_RETURN_ARG(QVariant, retval));
            });
        });
    }
    Q_INVOKABLE void exit_process() {
        exit(0);
    }
private:
    ProxySearch() { }
    QList<QFuture<void> > futures;
    QVector<ProxyListItem> max_runlist;
    QThreadPool search_threadpool;
    QThreadPool validate_threadpool;
    QQueue<QNetworkProxy> proxy_list;
    QVector<QString> ip_list;
    QStringList ssr_list;
    QQmlApplicationEngine* engine;
    QSet<QString> proxy_cache;
    QString search_src;
    QString dns_src;
    QString validate_src;
    QString validate_key;
    QMutex mlock;
    QObject* root_proxyfinder;
    QObject* root_dns_resolve;
    QObject* root_https_search;
    QObject* root_ssr_search;
    int lastcheck_dns;
    int lastcheck_proxy;
    int search_tmout;
    int validate_tmout;
    int validate_count;
    int thread_count;
    int httpserv_port;
    bool fulldns;
    bool is_search_done;
    bool is_searching;
    bool is_validating;
    bool is_validate_done;
    bool is_cancel;
};

class ProxySearchThread : public QObject, public QRunnable {
    Q_OBJECT
public:
    ProxySearchThread(ProxySearch* proxycls, QString url, QString root, QString host,
                      QString port, QString type) : url(url), root(root), host(host),
        port(port), type(type), proxycls(proxycls) {}
    void run();
private:
    QString url;
    QString root;
    QString host;
    QString port;
    QString type;
    ProxySearch* proxycls;
};

class ProxyValidateThread : public QObject, public QRunnable {
    Q_OBJECT
public:
    ProxyValidateThread(ProxySearch* proxycls, const QNetworkProxy& proxy, const QString& ip) :
        ip(ip), proxy(proxy), proxycls(proxycls) {
        hostname = proxy.hostName();
        port = proxy.port();
    }
//protected:
    void run();
private:
    QString ip;
    QString hostname;
    quint16 port;
    QNetworkProxy proxy;
    ProxySearch* proxycls;
};

class WorkThread : public QThread {
    Q_OBJECT
public:
    WorkThread(ProxySearch* proxycls) : proxycls(proxycls) {}
protected:
    void run() {
        proxycls->test_proxy_server_list();
    }
private:
    ProxySearch* proxycls;
};

#endif // PROXYSEARCHER_H
