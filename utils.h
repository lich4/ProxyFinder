#ifndef UTIL_H
#define UTIL_H

#include "common.h"
#include <libtidy/tidy.h>
#include <libtidy/tidybuffio.h>
#include <libtidy/tidyenum.h>
#include <libtidy/tidyplatform.h>

QString base64_decode(const QString& text);
QString base64_encode(const QString& text);

int GetHttpRequest(const QString& url, QString& response, const QString& method, const QString& postdata,
                   const QNetworkProxy& proxy, int& timeout);

int GetHttpRequestNoProxy(const QString& url, QString& response, int timeout, const QString& method = "get",
                               const QString& postdata = "");

QString tidy_html(const QString& content);

bool is_server_port_open (const QString& hostname, quint16 port, int timeout);

void log(const QString& content);

void printxml(pugi::xml_node node, int depth);

#endif // UTIL_H
