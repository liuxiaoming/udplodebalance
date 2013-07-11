#-------------------------------------------------
#
# Project created by QtCreator 2013-07-03T22:11:35
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Loder
TEMPLATE = app


SOURCES += main.cpp\
        loder.cpp \
    udpsockets.cpp

HEADERS  += loder.h \
    headFile.h \
    udpsockets.h \
    msg.h

FORMS    += loder.ui

OTHER_FILES += \
    configuration_loader.txt \
    log.txt
