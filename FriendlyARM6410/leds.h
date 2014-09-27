#ifndef LEDS_H
#define LEDS_H

#include <QObject>

class Leds : public QObject
{
    Q_OBJECT
public:
    explicit Leds(QObject *parent = 0);

signals:

public slots:

};

#endif // LEDS_H
