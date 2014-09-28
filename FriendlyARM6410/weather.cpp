#include "weather.h"

#include <QFile>
#include <QXmlStreamReader>
#include <QDebug>
#include <QtNetwork>

Weather::Weather(QObject *parent) :
    QThread(parent)
{
    manager = new QNetworkAccessManager(this);
    connect(manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(replyFinished(QNetworkReply*)));
}

Weather::~Weather()
{
    delete manager;
}

void Weather::getCityWeather(QString city)
{
    QString weatherUrl="http://flash.weather.com.cn/wmaps/xml/"+city+".xml";
    qDebug()<<"url: "<<weatherUrl;
    manager->get(QNetworkRequest(QUrl(weatherUrl)));
}

QString Weather::getWeatherString()
{
    if(!citys.contains("西安市")){
        return "无法获得 天气信息！";
    }
    QString humidity = citys.value("西安市").value("humidity").toString();
    //qDebug()<<humidity;
    QString tmp = QString("西安市 %1, 最高气温 %2度，最低气温 %3度, 当前温度 %4度, %5, %6  %7, 湿度 百分之%8")
            .arg(citys.value("西安市").value("stateDetailed").toString())
            .arg(getSpeachNum(citys.value("西安市").value("tem1").toString().toInt()))
            .arg(getSpeachNum(citys.value("西安市").value("tem2").toString().toInt()))
            .arg(getSpeachNum(citys.value("西安市").value("temNow").toString().toInt()))
            .arg(citys.value("西安市").value("windState").toString())
            .arg(citys.value("西安市").value("windDir").toString())
            .arg(citys.value("西安市").value("windPower").toString())
            .arg(getSpeachNum(humidity.left(humidity.size()-1).toInt()));
    return tmp;
}

QString Weather::getSpeachNum(int num)
{
    QString ret;
    //范围为 -99 ～ 99
    if(num > 0){
        if(num>10){
            if(num>19){
                ret.append(QString::number(num/10)).append("十").append(QString::number(num%10));
            }else{
                ret.append("十").append(QString::number(num%10));
            }
        }else if(num < 10){
            ret = QString(num);
        }else{
            ret = QString("十");
        }
    }else if(num < 0){
        if(num < -10){
            if(num < -19){
               ret.append("零下").append(QString::number(num/-10)).append("十").append(QString::number(num%-10));
            }else{
                ret.append("零下").append("十").append(QString::number(num%-10));
            }
        }if(num > -10){
            ret.append("零下").append(QString::number(num%-10));
        }else{
            ret.append("零下十");
        }
    }else{
        ret = "零";
    }
    qDebug()<<num<<" to "<<ret;
    return ret;
}

void Weather::replyFinished(QNetworkReply *reply)
{
    QXmlStreamReader reader;
    //设置文件，这时会将流设为初始状态
    reader.setDevice(reply);
    //如果没有读到文件的结尾，且没有出现错误
    while (! reader.atEnd()) {
        //读取下一个记号，它返回记号的类型
        QXmlStreamReader::TokenType type = reader.readNext();
        //根据记号的类型来进行不同的输出
        if(type == QXmlStreamReader::StartDocument){
            qDebug()<<"startDocument:"<<reader.documentEncoding()<<reader.documentVersion();
        }
        if(type == QXmlStreamReader::StartElement){
            qDebug()<<"startElement: <"<<reader.name()<<">";
            if(reader.attributes().hasAttribute("dn")){
                day = reader.attributes().value("dn").toString();
            }
            if(reader.attributes().hasAttribute("cityname")){
                qDebug()<<reader.attributes().value("cityname");
                citys.insert(reader.attributes().value("cityname").toString(),reader.attributes());
            }
        }
        if(type == QXmlStreamReader::EndElement){
            qDebug()<<"endElement: </"<<reader.name()<<"/>";
        }
        if(type == QXmlStreamReader::Characters && ! reader.isWhitespace()){
            qDebug()<<"text:"<<reader.text();
        }
    }
    if(reader.hasError()){
        qDebug()<<"error:"<<reader.errorString();
    }

    emit dataUpdate();                  //发送更新信号
    QString res =QString("speak -vzh+f3 -p 80 '").append(getWeatherString()).append("'");
    qDebug()<<res;
    ::system(res.toUtf8());
    reply->deleteLater();
}

void Weather::run()
{
    //可用于更新数据
    while (1) {

    }
}
