#include "vncserver.h"
#include <string.h>
#include <time.h>
#include <stdio.h>

VNCServer::VNCServer(QObject *parent) :
    QThread(parent)
{
}

VNCServer::VNCServer(int argc, char **argv, QObject *parent) :
    QThread(parent)
{
    this->argc = argc;
    this->argv = argv;
    this->picture_timeout = 1.0/15.0;  //15帧每秒，如果可以
    this->width = QApplication::desktop()->width();
    this->height = QApplication::desktop()->height();
#ifdef ARM_LINUX    //嵌入式 depth 是 16 位 16-bit RGB format (5-6-5)
    this->bpp = 2;
    server = rfbGetScreen(&this->argc,this->argv,width,height,5,3,bpp);
#elif defined DESKTOP
    this->bpp = 4;
    server = rfbGetScreen(&this->argc,this->argv,width,height,8,3,bpp);
#endif
    if(!server){
        qDebug()<<"get vnc screen failed!";
        return;
    }
    server->alwaysShared = true;
    server->desktopName = "X-AI";
    //frameBuffer 横向扫描
    server->frameBuffer = (char*)malloc(width*height*bpp);
}

void VNCServer::stopServer()
{
    if(rfbIsActive(server)){
        rfbShutdownServer(server,true);
    }
}

int VNCServer::timeToTakePicture()
{
    static struct timeval now={0,0}, then={0,0};
    double elapsed, dnow, dthen;

    gettimeofday(&now,NULL);

    dnow = now.tv_sec + (now.tv_usec/1000000.0);
    dthen = then.tv_sec + (then.tv_usec/1000000.0);
    elapsed = dnow - dthen;

    if(elapsed > this->picture_timeout)
        memcpy((char*)&then,(char*)&now,sizeof(struct timeval));

    return elapsed > this->picture_timeout;
}

int VNCServer::takePicture()
{
//实践：QImage和FrameBuffer中的像素数据，和libvnc中不同，红蓝要互换位置，QImage 为Format_RGB32，Vinagre使用JPEG压缩，不互换位置的可以正常显示
#ifdef DESKTOP
    QImage img = qApp->primaryScreen()->grabWindow(0).toImage();  //QImage::Format_RGB32
    u_int32_t* f = (u_int32_t*)img.bits();
    for(int i=0;i<width*height;++i){
        f[i] = ((f[i] & 0x00ff0000))>>16 | ((f[i] & 0x0000ff00)) | ((f[i] & 0x000000ff))<<16; //0xffBBGGRR
    }
#elif defined ARM_LINUX
    QImage img = QImage(QDirectPainter::frameBuffer(),width,height,QImage::Format_RGB16);
    img = img.convertToFormat(QImage::Format_RGB555);   //转换成VNC支持的格式
    u_int16_t* f = (u_int16_t*)img.bits();
    for(int i=0;i<width*height;++i){
        f[i] = ((f[i] & 0b0111110000000000))>>10 | ((f[i] & 0b0000001111100000)) | ((f[i] & 0b0000000000011111))<<10; //0b0(blue)(green)(red)
    }
#endif
    if(!img.isNull() && img.byteCount()>0){
        memcpy(server->frameBuffer,img.bits(),width*height*bpp);
    }else{
        qDebug()<<"get image failed!!\r";
        return false;
    }
    return true;
}

void VNCServer::run()
{
    long usec;
    //初始化服务
    rfbInitServer(server);
    qDebug()<<"bpp:"<<server->serverFormat.bitsPerPixel
            <<" depth:"<<server->serverFormat.depth
            <<" bigEndian:"<<server->serverFormat.bigEndian
            <<" trueColour:"<<server->serverFormat.trueColour
            <<" redMax:"<<server->serverFormat.redMax
            <<" greenMax:"<<server->serverFormat.greenMax
            <<" blueMax:"<<server->serverFormat.blueMax
            <<" deferUpdateTime:"<<server->deferUpdateTime;

    //循环处理事件
    while(rfbIsActive(server)){
        if(timeToTakePicture()){
            if(takePicture()){
                rfbMarkRectAsModified(server,0,0,width,height);
            }
        }
        usec = server->deferUpdateTime * 1000;
        rfbProcessEvents(server,usec);
    }
}
