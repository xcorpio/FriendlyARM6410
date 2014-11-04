#ifndef WEATHER_H
#define WEATHER_H

#include <QThread>
#include <QMap>
#include <QXmlStreamAttributes>

class QNetworkReply;
class QNetworkAccessManager;

class Weather: public QObject
{
    Q_OBJECT
public:
    explicit Weather(QObject* parent = 0);
    ~ Weather();

    QString getWeatherString();             //把天气信息转换成字符串
    QString getSpeachNum(int num);          //例  25 =》》 二十五
signals:
    void dataUpdate();      //信息更新发送

public slots:
    void speakWeather();    //播放天气声音
    void getCityWeather(QString city);

private slots:
    void replyFinished(QNetworkReply *reply);

    // QThread interface
protected:
    void run();

private:
    QNetworkAccessManager * manager;
public:
    QString day;                        //day or night
    QMap<QString,QXmlStreamAttributes> citys;

};

#endif // WEATHER_H
