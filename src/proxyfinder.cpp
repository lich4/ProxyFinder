#include "proxysearcher.h"
#include "utils.h"

// https://prom-php.herokuapp.com/cloudfra_ssr.txt
// https://raw.githubusercontent.com/ImLaoD/sub/master/ssrshare.com
// https://yzzz.ml/freessr/

// https://gdmi.weebly.com/3118523398online.html
// //tr[@class='wsite-multicol-tr']//a//@href
// xpathnode.attribute().value()

// http://ss.pythonic.life/\1-\2 ->    \2失败则\1++  \1失败且\2为0则跳出
// //input/@value
// xpathnode.attribute().value()

// http://ss.pythonic.life/subscribe
// https://www.ssrtool.com/tool/free_ssr
// https://www.ssrtool.com/tool/free_ssrC
// https://x.ishadowx.net/


void testxpath() {
    QString root = "//section[@class='carousel']//a/@href";
    QString host = "td[1]/text()";
    QString port = "td[2]/text()";
    QString content;
    pugi::xml_document doc;
    int timeout = 10000;
    int i = 0, j, isend = false;
    while (!isend) {
        j = 0;
        while (true) {
            content = "";
            QString url = QString("https://ss.pythonic.life/%1-%2").arg(i).arg(j);
            GetHttpRequestNoProxy(url, content, timeout);
            doc.load_string(tidy_html(content).toLatin1().data());
            pugi::xpath_node xpathnode = doc.select_node("//input/@value");
            if (xpathnode.node().empty()) {
                if (j == 0) {
                    isend = true;
                }
                break;
            }
            qDebug() << xpathnode.attribute().value();
        }
    }
}


int main(int argc, char *argv[])
{
    // QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    app.setFont(QFont("Microsoft Yahei", 9));
    app.setWindowIcon(QIcon(":/images/qt-logo.ico"));


    //testxpath();

    QQmlApplicationEngine engine;
    QPM_INIT(engine)
    engine.load(QUrl("qrc:/proxyfinder.qml"));
    engine.rootContext()->setContextProperty("commu", ProxySearch::getinst(&engine));
    if (engine.rootObjects().isEmpty())
        return -1;
    return app.exec();
}
