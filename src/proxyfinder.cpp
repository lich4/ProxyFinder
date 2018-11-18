#include "proxysearcher.h"
#include "utils.h"

/*
void testxpath() {
    QString root = "//table//tr[position()>1]";
    QString host = "td[1]/text()";
    QString port = "td[2]/text()";
    QString content;
    GetHttpRequestNoProxy("https://github.com/", content, 10000);
    log(content);
    content = tidy_html(content);
    pugi::xml_document doc;
    doc.load_string(content.toLatin1().data());

    pugi::xpath_node_set xpathnodes = doc.select_nodes(root.toLatin1().data());
    for (const pugi::xpath_node& xpathnode : xpathnodes) {
        printxml(xpathnode.node(), 0);
        const pugi::xml_node& xmlnode = xpathnode.node();
        QString xx = xmlnode.name();
        pugi::xpath_node hostnode = xmlnode.select_node(host.toLatin1().data());
        pugi::xpath_node portnode = xmlnode.select_node(port.toLatin1().data());
        QString host = hostnode.node().value();
        quint16 port = static_cast<quint16>(QString(portnode.node().value()).toInt());
        qDebug() << host;
    }
}
*/

int main(int argc, char *argv[])
{
    // QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    app.setFont(QFont("Microsoft Yahei", 9));
    app.setWindowIcon(QIcon(":/images/qt-logo.ico"));


    // testxpath();

    QQmlApplicationEngine engine;
    QPM_INIT(engine)
    engine.load(QUrl("qrc:/proxyfinder.qml"));
    engine.rootContext()->setContextProperty("commu", ProxySearch::getinst(&engine));
    if (engine.rootObjects().isEmpty())
        return -1;
    return app.exec();
}
