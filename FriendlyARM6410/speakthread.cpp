#include "speakthread.h"
#include <QDebug>

SpeakThread::SpeakThread(QObject *parent) :
    QThread(parent)
{
}

SpeakThread::SpeakThread(QString words)
{
    this->words = words;
}

void SpeakThread::run()
{
    QString res =QString("speak -vzh+f3 -p 80 -s 150 '").append(words).append("'");
    qDebug()<<res.toUtf8();
    ::system(res.toLatin1());
}
