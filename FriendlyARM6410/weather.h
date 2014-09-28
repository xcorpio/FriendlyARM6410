#ifndef WEATHER_H
#define WEATHER_H

#include <QThread>
#include <QMap>
#include <QXmlStreamAttributes>

class QNetworkReply;
class QNetworkAccessManager;

class Weather : public QThread
{
    Q_OBJECT
public:
    explicit Weather(QObject *parent = 0);
    ~ Weather();
    void getCityWeather(QString city);
    QString getWeatherString();             //把天气信息转换成字符串
    QString getSpeachNum(int num);          //例  25 =》》 二十五
signals:
    void dataUpdate();      //信息更新发送

public slots:
private slots:
    void replyFinished(QNetworkReply *reply);

    // QThread interface
protected:
    void run();

private:
    bool flag;                          //循环标志
    QString day;                        //day or night
    QNetworkAccessManager * manager;
    QMap<QString,QXmlStreamAttributes> citys;
};

#endif // WEATHER_H
