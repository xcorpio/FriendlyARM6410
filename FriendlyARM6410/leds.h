#ifndef LEDS_H
#define LEDS_H

#include <QObject>


class Leds : public QObject
{

public:
    explicit Leds(QObject *parent = 0);
    ~Leds();
    static void setLedState(int ledID, int ledState);   //ledID:0-3 ledState:1 on,0 off

private:
    static int m_fd;

signals:


public slots:
    static void turnAllOn();
    static void turnAllOff();
};

#endif // LEDS_H
