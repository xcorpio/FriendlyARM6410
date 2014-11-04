#include "buttons.h"
#include <sys/stat.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <QDebug>
#include <iostream>

using namespace std;

Buttons::Buttons(QObject *parent) :
    QObject(parent)
{
    m_fd = ::open("/dev/buttons",O_RDONLY | O_NONBLOCK);
    if(m_fd < 0){
        qDebug()<<"open /dev/buttons failed!";
        return;
    }
    notifier = new QSocketNotifier(m_fd,QSocketNotifier::Read,parent);
    connect(notifier,SIGNAL(activated(int)),this,SLOT(buttonClikedTest(int)));
    qDebug()<<"notifier: "<<notifier->isEnabled();
}

Buttons::~Buttons()
{
    delete notifier;
    if(m_fd > 0){
        ::close(m_fd);
    }
}

void Buttons::buttonClikedTest(int fd)
{
    //qDebug()<<"some can be read!";
    char buffer[8];
    memset(buffer,0,sizeof(buffer));
    ::read(fd,buffer,sizeof(buffer));
    for(uint i=0;i<sizeof(buffer)/sizeof(buffer[0]);++i){
        this->on[i] = (buffer[i] & 0x01);
        emit buttonsClicked(this->on);
    }
}
