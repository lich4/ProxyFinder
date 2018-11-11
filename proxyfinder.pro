TEMPLATE = app

TARGET = proxyfinder

QT += quick widgets xmlpatterns concurrent sql

CONFIG += c++11

SOURCES += \
    proxyfinder.cpp \
    utils.cpp \
    pugixml.cpp \
    proxysearcher.cpp

RESOURCES += \
    proxyfinder.qml \
	config.qml \
    $$files(images/*.*) 

RC_ICONS = qt-logo.ico

INSTALLS += target

HEADERS += \
    proxysearcher.h \
    common.h \
    utils.h

DISTFILES +=

win32: LIBS += -L$$PWD/./ -ltidy -L$$PWD/./ -lxml2

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.

target.path = $$PWD/debug
