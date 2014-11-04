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
