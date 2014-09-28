#-------------------------------------------------
#
# Project created by QtCreator 2014-09-25T15:31:38
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FriendlyARM6410
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    leds.cpp \
    weather.cpp

HEADERS  += mainwindow.h \
    leds.h \
    weather.h

FORMS    += mainwindow.ui
