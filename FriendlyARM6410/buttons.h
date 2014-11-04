#ifndef BUTTONS_H
#define BUTTONS_H

#include <QObject>
#include <QSocketNotifier>

class Buttons : public QObject
{
    Q_OBJECT
public:
    explicit Buttons(QObject *parent = 0);
    ~Buttons();

signals:
    void buttonsClicked(bool* on);

public slots:
    void buttonClikedTest(int fd);
private:
    int m_fd;
    QSocketNotifier* notifier;
    bool on[8];         //存放当前按键状态

};

#endif // BUTTONS_H
