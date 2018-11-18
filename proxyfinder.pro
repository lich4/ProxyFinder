DEFINES += QPM_INIT\\(E\\)=\"E.addImportPath(QStringLiteral(\\\"qrc:/\\\"));\"
include(material/material.pri)

QT += quick widgets xmlpatterns concurrent sql
CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    src/proxyfinder.cpp \
    src/utils.cpp \
    src/pugixml.cpp \
    src/proxysearcher.cpp

RESOURCES += \
    icons/icons.qrc \
    images/images.qrc \
    qml/qml.qrc

RC_ICONS = images/qt-logo.ico


HEADERS += \
    src/proxysearcher.h \
    src/common.h \
    src/utils.h

win32: LIBS += -L$$PWD/lib/ -ltidy -L$$PWD/lib/ -lxml2

INCLUDEPATH += $$PWD/src
DEPENDPATH += $$PWD/lib
