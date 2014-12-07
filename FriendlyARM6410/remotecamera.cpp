#include "remotecamera.h"
#include "convert.h"
#include "lcd.h"

#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <syslog.h>
#include <time.h>
#include <signal.h>
#include <QDebug>
#include <QImage>
#include <QTime>

static int force_exit = 0;

static int my_protocol_callback(struct libwebsocket_context* context,
                     struct libwebsocket* wsi,
                     enum libwebsocket_callback_reasons reason,
                     void* user, void* in, size_t len)
{
    switch(reason){

    case LWS_CALLBACK_ESTABLISHED:
        qDebug()<<"New Connection.";
        break;
    case LWS_CALLBACK_SERVER_WRITEABLE:
        qDebug()<<"server wirteable.";
        break;
    case LWS_CALLBACK_RECEIVE:
        qDebug()<<"Got:"<<(char*)in;
        break;
    case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
        qDebug()<<"filter protocol.";
        break;
    case LWS_CALLBACK_CLIENT_ESTABLISHED:
        qDebug()<<"Client has connected";
        break;
    case LWS_CALLBACK_CLIENT_RECEIVE:
        //qDebug()<<"Client Rx("<<len<<"): ";
        RemoteCamera::drawFb(in);   //写入FrameBuffer
        break;
    case LWS_CALLBACK_CLIENT_WRITEABLE:
        //send our packet..

        break;
    default:
        break;
    }

    return 0;
}

//lsit of supported protocols and callbacks

static struct libwebsocket_protocols protocols[] = {
    {
        "default",
        my_protocol_callback,0,460800
    },
    {NULL,NULL,0,0} //terminator
};

//char* address = "127.1.1.1";
static char* address = "192.168.2.5";
static int port = 8888;
static int fps = 0;
static int m_fd2;   //   /dev/fb2
static char* fb_addr;

RemoteCamera::RemoteCamera(QObject *parent) :
    QThread(parent)
{

}

void RemoteCamera::drawFb(void *in)
{
    //==================fps
    static struct timeval now={0,0}, then={0,0};
    static int count = 0;
    double elapsed, dnow, dthen;

    gettimeofday(&now,NULL);

    dnow = now.tv_sec + (now.tv_usec/1000000.0);
    dthen = then.tv_sec + (then.tv_usec/1000000.0);
    elapsed = dnow - dthen;

    //计算帧数
    count++;
    if(elapsed > 1.0){
        fps = count;
        count = 0;
        memcpy((char*)&then,(char*)&now,sizeof(struct timeval));
    }

    //==================

    int t = QTime::currentTime().msec();
    QImage img = QImage(320,240,QImage::Format_ARGB32);
    qt_convert_NV21_to_ARGB32((uchar*)in,(quint32*)img.bits(),320,240);
    img = img.convertToFormat(QImage::Format_RGB16);
    for(int i=0;i<240;++i){
        memcpy(fb_addr, img.bits(), 320*240*2);     //写到fb2
    }
    t = QTime::currentTime().msec() - t;
    if(t<0) t+=1000;
    fprintf(stderr,"fb need time: %d ms  fps: %03d\r", t, fps); //120ms
}

void RemoteCamera::openCamera()
{
    if(!this->isRunning()){
        m_fd2 = open(FB_DEV_NAME2, O_RDWR | O_NDELAY);
        if (m_fd2 >= 0) {
            s3c_win_info_t fb_info_to_driver;
            fb_info_to_driver.Bpp = 16;  //BPP
            fb_info_to_driver.LeftTop_x = 150;
            fb_info_to_driver.LeftTop_y = 30;
            fb_info_to_driver.Width = 320;
            fb_info_to_driver.Height = 240;

            int fb_size = fb_info_to_driver.Width * fb_info_to_driver.Height * 2; // RGB565
            fb_addr = (char *) mmap(0, fb_size, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd2, 0);
            if (fb_addr == NULL) {
                ::close(m_fd2);
                m_fd2 = -1;
                 qDebug("mmap fb2 failed!");
                return ;
            }

            if (ioctl(m_fd2, SET_OSD_INFO, &fb_info_to_driver) == -1) {
                qDebug("SET_OSD_INFO fb2 failed!");
            }

            if (ioctl(m_fd2, SET_OSD_START) == -1) {
                qDebug("SET_OSD_START fb2 failed!");
            }
        } else {
            qDebug("open fb2 failed!");
        }

        //start thread
        this->start();
        force_exit = 0;
    }
}

void RemoteCamera::closeCamera()
{
    if(context){
        force_exit = 1;
    }
    if(m_fd2){
        if (ioctl(m_fd2, SET_OSD_STOP) == -1) {
            qDebug("SET_OSD_STOP fb2 failed!");
        }
    }
}

void RemoteCamera::stopPreview()
{
    if(m_fd2){
        if (ioctl(m_fd2, SET_OSD_STOP) == -1) {
            qDebug("SET_OSD_STOP fb2 failed!");
        }
    }
}

void RemoteCamera::restorePreview()
{
    if (ioctl(m_fd2, SET_OSD_START) == -1) {
        qDebug("SET_OSD_START fb2 failed!");
    }
}

void RemoteCamera::run()
{
    context = NULL;
    wsi = NULL;
    int n = 0;
    int opts = 0;
    const char* iface = NULL;
    int syslog_options = LOG_PID | LOG_PERROR;

    struct lws_context_creation_info info;

    memset(&info,0,sizeof(info));
    //signal(SIGINT, sighandler);

    //only try to log things according to our debug_level
    setlogmask(LOG_UPTO(LOG_DEBUG));
    openlog("lwsts",syslog_options,LOG_DAEMON);

    //tell the library what debug level to emit and to send it to syslog
    info.port = CONTEXT_PORT_NO_LISTEN; //as a client
    info.iface = iface;
    info.protocols = protocols;
    info.extensions = libwebsocket_get_internal_extensions();
    info.ssl_cert_filepath = NULL;
    info.ssl_private_key_filepath = NULL;
    info.gid = -1;
    info.uid = -1;
    info.options = opts;

    context = libwebsocket_create_context(&info);
    qDebug()<<"Create context finished.";

    //connect server
    wsi = libwebsocket_client_connect(context,address,port,0,"/",address,"origin",NULL,-1);
    if(!wsi){
        qDebug()<<"connect server failed.";
        return;
    }

    n = 0;
    while ( n>=0 && !force_exit) {
        //it does the polling and if threr is no transmission it returns after 50 ms.
        n = libwebsocket_service(context,50);
    }

    qDebug()<<"libwebsocket context will destroy";
    libwebsocket_context_destroy(context);

    lwsl_notice("libwebsockets-test-server exit cleanky \n");

    closelog();
}

