#-------------------------------------------------
#
# Project created by QtCreator 2013-07-05T10:49:09
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Server
TEMPLATE = app


SOURCES += main.cpp\
        server.cpp \
    udpsockets.cpp

HEADERS  += server.h \
    udpsockets.h \
    msg.h \
    headFile.h

FORMS    += server.ui
