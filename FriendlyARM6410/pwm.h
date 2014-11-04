#ifndef PWM_H
#define PWM_H

#include <QObject>

class PWM : public QObject
{
    Q_OBJECT
public:
    explicit PWM(QObject *parent = 0);
    ~PWM();

signals:

public slots:
    void setFreq(int freq);
    void stopPWM();
private:
    int m_fd;

};

#endif // PWM_H
