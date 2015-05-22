exists($$PWD/../conf.pri):include($$PWD/../conf.pri)

QT -= gui
QT += network

INCLUDEPATH += $$PWD/../src
LIBS += -L$$PWD/../src -lqfiber
PRE_TARGETDEPS += $$PWD/../src/libqfiber.a
