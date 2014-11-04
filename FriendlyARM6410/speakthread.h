#ifndef SPEAKTHREAD_H
#define SPEAKTHREAD_H

#include <QThread>

class SpeakThread : public QThread
{
    Q_OBJECT
public:
    explicit SpeakThread(QObject *parent = 0);
    SpeakThread(QString words);

signals:

public slots:


    // QThread interface
protected:
    void run();

private:
    QString words;
};

#endif // SPEAKTHREAD_H
