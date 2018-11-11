#ifndef PROXYSEARCHER_H
#define PROXYSEARCHER_H

#include "common.h"
#include "utils.h"

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
            inst.root_item = engine->rootObjects().first();

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

    void update_status(QString txt) {
        FunctionTransfer::execinmain([&](){
            QVariant retval;
            QVariant arg0 = txt;
            QMetaObject::invokeMethod(root_item, "update_status", Qt::DirectConnection,
                Q_RETURN_ARG(QVariant, retval), Q_ARG(QVariant, arg0));
        });
    }

    void update_runlist(int x, int y, QString txt) {
        FunctionTransfer::execinmain([&](){
            QVariant retval;
            QVariant arg0 = x;
            QVariant arg1 = y;
            QVariant arg2 = txt;
            QMetaObject::invokeMethod(root_item, "update_runlist", Qt::DirectConnection,
                Q_RETURN_ARG(QVariant, retval),
                Q_ARG(QVariant, arg0), Q_ARG(QVariant, arg1), Q_ARG(QVariant, arg2));
        });
    }

    void do_dns_lookup() {
        update_status("根据DNS查找IP");
        QString validate_host = QUrl(validate_src).host();
        int p = validate_src.indexOf(validate_host);
        QString pre = validate_src.mid(0, p);
        QString post = validate_src.mid(p + validate_host.length());
        if (fulldns) {
            int total = 0;
            int handle = 0;
            QString res;
            GetHttpRequestNoProxy(dns_src, res, 10000);
            QList<QFuture<void> > futures;
            auto lambda = [&] (QString host) {
                QDnsLookup dns;
                QEventLoop loop;
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
                    update_status(QString("发现%1的IP:%2 %3/%4").arg(validate_host).arg(ip)
                                               .arg(handle).arg(total));
                    ip_list.insert(pre + ip + post);
                    mlock.unlock();
                }
                mlock.lock();
                handle++;
                mlock.unlock();
            };
            QStringList lines = res.split("\n");
            total = lines.size();
            for (QString& line : lines) {
                futures.append(QtConcurrent::run(lambda, line));
            }
            for(auto future : futures) {
                future.waitForFinished();
            }
        } else {
            QDnsLookup dns;
            QEventLoop loop;
            QObject::connect(&dns, &QDnsLookup::finished, &loop, &QEventLoop::quit);
            dns.setType(QDnsLookup::A);
            dns.setName(validate_host);
            dns.lookup();
            loop.exec();
            if (dns.error() != QDnsLookup::NoError) {
                return;
            }
            foreach (const QDnsHostAddressRecord &record, dns.hostAddressRecords()) {
                QString ip = record.value().toString();
                update_status(QString("发现%1的IP:%2").arg(validate_host).arg(ip));
                mlock.lock();
                ip_list.insert(pre + ip + post);
                mlock.unlock();
            }
        }
        update_status("DNS查找完毕");
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
    Q_INVOKABLE void set_search_timeout(const QString& s) {
        search_tmout = s.toInt();
    }
    Q_INVOKABLE QString get_search_timeout() {
        return QString("%1").arg(search_tmout);
    }
    Q_INVOKABLE void set_validate_timeout(const QString& s) {
        validate_tmout = s.toInt();
    }
    Q_INVOKABLE QString get_validate_timeout() {
        return QString("%1").arg(validate_tmout);
    }
    Q_INVOKABLE void set_validate_count(const QString& s) {
        validate_count = s.toInt();
    }
    Q_INVOKABLE QString get_validate_count() {
        return QString("%1").arg(validate_count);
    }
    Q_INVOKABLE void set_thread_count(const QString& s) {
        thread_count = s.toInt();
        validate_threadpool.setMaxThreadCount(thread_count);
    }
    Q_INVOKABLE QString get_thread_count() {
        return QString("%1").arg(thread_count);
    }
    Q_INVOKABLE void set_fulldns(bool checked) {
        fulldns = checked;
    }
    Q_INVOKABLE void do_search_proxy() {
        if (is_searching || is_validating) {
            return;
        }
        // 清空
        for (int i = 0; i < MAX_LNE_NUM; i++) {
            for (int j = 0; j < MAX_COL_NUM; j++) {
                update_runlist(i, j, "------------");
            }
        }
        update_status(QString("搜索代理:进行 验证代理:停止"));
        update_status("开始搜索");
        is_cancel = false;
        ip_list.clear();
        QtConcurrent::run([&]() {
            int ret = 0;
            // 从hosname获取ip列表
            do_dns_lookup();
            if (ip_list.size() == 0) {
                update_status(QString("搜索代理:进行 验证代理:停止 IP获取失败"));
                return;
            }
            // 从代理源获取代理列表
            if (0 != (ret = get_proxy_server_list())) {
                update_status(QString("搜索代理:进行 验证代理:停止 代理列表获取失败"));
                return;
            }
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
            mlock.lock();
            validate_threadpool.setExpiryTimeout(0);
            search_threadpool.setExpiryTimeout(0);
            proxy_list.clear();
            max_runlist.clear();
            search_threadpool.clear();
            validate_threadpool.clear();
            mlock.unlock();
        }
        is_searching = false;
        is_validating = false;
        update_status(QString("搜索代理:进行 验证代理:停止 已取消搜索"));
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
    Q_INVOKABLE void exit_process() {
        cancel_search_proxy();
        exit(0);
    }
private:
    ProxySearch() { }
    QList<QFuture<void> > futures;
    QVector<ProxyListItem> max_runlist;
    QThreadPool search_threadpool;
    QThreadPool validate_threadpool;
    QQueue<QNetworkProxy> proxy_list;
    QSet<QString> ip_list;
    QQmlApplicationEngine* engine;
    QSet<QString> proxy_cache;
    QString search_src;
    QString dns_src;
    QString validate_src;
    QString validate_key;
    QMutex mlock;
    QObject* root_item;
    int search_tmout;
    int validate_tmout;
    int validate_count;
    int thread_count;
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
    WorkThread(ProxySearch* proxycls) : proxycls(proxycls) {};
protected:
    void run() {
        proxycls->test_proxy_server_list();
    }
private:
    ProxySearch* proxycls;
};

#endif // PROXYSEARCHER_H
