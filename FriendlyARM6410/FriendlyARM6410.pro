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
    weather.cpp \
    speakthread.cpp \
    buttons.cpp \
    pwm.cpp \
    vncserver.cpp

HEADERS  += mainwindow.h \
    leds.h \
    weather.h \
    speakthread.h \
    buttons.h \
    pwm.h \
    vncserver.h

FORMS    += mainwindow.ui

RESOURCES += \
    res.qrc

QMAKE_CXXFLAGS += -Wno-psabi -Wno-write-strings

#desktop
linux-g++ {
    message(Linux)
    DEFINES += DESKTOP
    LIBS += -lvncserver
}

#embeded
linux-arm-g++ {
    message(arm-linux)
    DEFINES += ARM_LINUX
    INCLUDEPATH += /opt/FriendlyARM/xcorpio/libvncserver-0.9.10/_install/include
    LIBS += -L/opt/FriendlyARM/xcorpio/libvncserver-0.9.10/_install/lib -lvncserver  -lturbojpeg -ljpeg \
                -lcrypt -lcrypto -lssl -lresolv
}
