#include "pwm.h"
#include <sys/stat.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <QDebug>

PWM::PWM(QObject *parent) :
    QObject(parent)
{
    m_fd = ::open("/dev/pwm",O_RDONLY);
    if(m_fd < 0){
        qDebug()<<"open /dev/pwm failed!";
        return;
    }
}

PWM::~PWM()
{
    ::close(m_fd);
}

void PWM::setFreq(int freq)
{
    ::ioctl(m_fd,1,freq);
}

void PWM::stopPWM()
{
    ::ioctl(m_fd,0);
}
