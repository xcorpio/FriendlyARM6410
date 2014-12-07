#include "leds.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

Leds::Leds(QObject *parent) :
    QObject(parent)
{
}

Leds::~Leds()
{
    ::close(m_fd);
}

int Leds::m_fd = ::open("/dev/leds",O_RDONLY);

void Leds::setLedState(int ledID, int ledState)
{
    ioctl(m_fd,ledState,ledID);
}

void Leds::turnAllOn()
{
    ioctl(m_fd,1,0);
    ioctl(m_fd,1,1);
    ioctl(m_fd,1,2);
    ioctl(m_fd,1,3);
}

void Leds::turnAllOff()
{
    ioctl(m_fd,0,0);
    ioctl(m_fd,0,1);
    ioctl(m_fd,0,2);
    ioctl(m_fd,0,3);
}
