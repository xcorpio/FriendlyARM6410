#ifndef VNCSERVER_H
#define VNCSERVER_H

#include <QThread>
#include <QImage>
#include <QDebug>
#include <QPixmap>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QCursor>

#ifdef ARM_LINUX
#include <QDirectPainter>
#include <QWSServer>
#endif

#include <rfb/rfb.h>
#include <rfb/rfbproto.h>
#include <rfb/keysym.h>
#include <fcntl.h>
#include <errno.h>

extern int fps;     //帧数

class VNCServer : public QThread
{
    Q_OBJECT
public:
    explicit VNCServer(QObject *parent = 0);
    VNCServer(int argc,char** argv,QObject *parent = 0);
    void stopServer();
signals:


public slots:


private:
    int argc;
    char** argv;

    int timeToTakePicture();            //根据时间是否该截图
    int takePicture();                  //填充framebuffer

public:
    rfbScreenInfoPtr server;
    double picture_timeout;     //每帧的时间
    int width;                  //显示宽高
    int height;
    int bpp;                    //每像素字节数

    // QThread interface
protected:
    void run();
};

#endif // VNCSERVER_H
