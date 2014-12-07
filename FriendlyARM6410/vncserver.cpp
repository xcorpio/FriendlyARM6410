#include "vncserver.h"
#include <string.h>
#include <time.h>
#include <stdio.h>


static void keyEvent(rfbBool down, rfbKeySym key, rfbClientPtr cl);         //按键事件
static void mouseEvent(int buttonMask, int x, int y, rfbClientPtr cl);      //鼠标事件
//static rfbBool checkPasswd(struct _rfbClientRec *cl, const char *encryptedPassWord, int len);   //检查密码
static rfbNewClientAction newClientHook(rfbClientPtr cl);                   //新客户端连接
static void clientGone(rfbClientPtr cl);                                    //客户端断开连接
static void displayHook(rfbClientRec *c);                                   //before a framebuffer update send

int fps;        //帧数

/* Here we create a structure so that every client has it's own pointer */

typedef struct ClientData {
  rfbBool oldButton;
  int oldx,oldy;
} ClientData;

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
    server->ptrAddEvent = mouseEvent;
    server->kbdAddEvent = keyEvent;
    server->newClientHook = newClientHook;
    server->displayHook = displayHook;
    server->authPasswdData = (void*)"passwd";   //存放加密密码的文件
    //server->passwordCheck = checkPasswd;

    //http server
    server->httpDir = "/www/vnc-webclients";
    server->httpEnableProxyConnect = true;

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

static void keyEvent(rfbBool down, rfbKeySym key, rfbClientPtr cl)
{
    qDebug()<<"key event ==> "<<" down:"<<down<<" keySym:"<< QString::number(key,16)<<"  "<<cl->host;
#ifdef ARM_LINUX
    if(key == XK_BackSpace){
        QWSServer::sendKeyEvent(key,Qt::Key_Backspace,Qt::NoModifier,down,false);
    }else if(key>=0x20 && key<0x100){
        QWSServer::sendKeyEvent(key,key,Qt::NoModifier,down,false);
    }
    QWidget* w = qApp->focusWidget();
    if(w){
        w->update();
    }
#endif
}

static void mouseEvent(int buttonMask, int x, int y, rfbClientPtr cl)
{

    ClientData* cd = (ClientData*)cl->clientData;
#ifdef ARM_LINUX
    if(buttonMask){        //buttonMask 0:没有按键 1:鼠标左键 2:鼠标中间 4:鼠标右键 8:向上滚轮 16:向下滚轮
        if(buttonMask == cd->oldButton ){//MouseMove
            //QWSServer::sendMouseEvent(QPoint(x,y),QWSServer::MouseMove);  //此事件太多影响效率
            qDebug()<<"mouse move event ==> "<<" button:"<<buttonMask<<" x:"<<x<<" y:"<<y<<"  from:"<<cl->host;
        }else{//MousePress
//            if(buttonMask == 1){
//                QWSServer::sendMouseEvent(QPoint(x,y),Qt::LeftButton);
//            }else if(buttonMask == 2){
//                QWSServer::sendMouseEvent(QPoint(x,y),Qt::MidButton);
//            }else if(buttonMask == 4){
//                QWSServer::sendMouseEvent(QPoint(x,y),Qt::RightButton);
//            }
            QWSServer::sendMouseEvent(QPoint(x,y),QWSServer::MouseRelease);
            qDebug()<<"mouse press event ==> "<<" button:"<<buttonMask<<" x:"<<x<<" y:"<<y<<"  from:"<<cl->host;
        }
    }else{
        if(cd->oldButton){//MouseRelease
            QWSServer::sendMouseEvent(QPoint(x,y),QWSServer::MousePress);
            qDebug()<<"mouse release event ==> "<<" button:"<<cd->oldButton<<" x:"<<x<<" y:"<<y<<"  from:"<<cl->host;
        }else{//mouse hang
            QCursor::setPos(x,y);
            //qDebug()<<"mouse hang event ==> "<<" button:"<<buttonMask<<" x:"<<x<<" y:"<<y<<"  from:"<<cl->host;
        }
    }
    QWidget* w = qApp->focusWidget();   //更新使显示效果
    if(w){
        w->update();
    }
#endif
    cd->oldx = x; cd->oldy = y; cd->oldButton = buttonMask;
    rfbDefaultPtrAddEvent(buttonMask,x,y,cl);
}


static rfbNewClientAction newClientHook(rfbClientPtr cl)
{
    cl->clientData = (void*)calloc(sizeof(ClientData),1);
    cl->clientGoneHook = clientGone;
    qDebug()<<"newClientHool --> authPasswdData: "<<(char*)cl->screen->authPasswdData;
    qDebug()<<"decry pass: "<<rfbDecryptPasswdFromFile((char*)cl->screen->authPasswdData);
    return RFB_CLIENT_ACCEPT;
}

//和默认检测相同
//rfbBool checkPasswd(struct _rfbClientRec *cl, const char *encryptedPassWord, int len){
//    qDebug()<<"checkPasswd --> checking the password!"<<"  encryptedpasswd:"<<encryptedPassWord<<" len:"<<len;

//    int i;
//    char *passwd=rfbDecryptPasswdFromFile((char*)cl->screen->authPasswdData);

//    if(!passwd) {
//        rfbErr("Couldn't read password file: %s\n",cl->screen->authPasswdData);
//        return(FALSE);
//    }
//    qDebug()<<"checkPassws --> get passwd : "<<passwd;
//    rfbEncryptBytes(cl->authChallenge, passwd);

//    /* Lose the password from memory */
//    for (i = strlen(passwd); i >= 0; i--) {
//        passwd[i] = '\0';
//    }

//    free(passwd);

//    if (memcmp(cl->authChallenge, encryptedPassWord, len) != 0) {
//        rfbErr("authProcessClientMessage: authentication failed from %s\n",
//               cl->host);
//        return(FALSE);
//    }
//    for(int i=0;i<len;++i){
//        qDebug()<<QString::number(cl->authChallenge[i],16)<<" = "<<QString::number(encryptedPassWord[i],16);
//    }
//    return(TRUE);
//}

static void clientGone(rfbClientPtr cl)
{
    free(cl->clientData);
    cl->clientData = NULL;
}

static void displayHook(rfbClientRec *c)
{
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
    //fprintf(stderr,"Picture to %s (%03d fps)\r", c->host, fps);
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
