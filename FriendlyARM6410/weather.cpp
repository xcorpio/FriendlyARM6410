#include "weather.h"
#include "speakthread.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QDebug>
#include <QtNetwork>


Weather::Weather(QObject* parent):QObject(parent = 0)
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
    qDebug()<<QTextCodec::codecForUtfText("西安市")->name();
    if(!citys.contains(QString::fromUtf8("西安市"))){
        return "无法获得 天气信息！";
    }

    //西安市 多云转阴, 最高气温 十6度，最低气温 2十5度, 当前温度 2十4度, 东北风小于3级, 东风  2级, 湿度 百分之5十6
    QString humidity = citys.value(QString::fromUtf8("西安市")).value("humidity").toString();
    QString stateDetailed = citys.value(QString::fromUtf8("西安市")).value("stateDetailed").toString();
    QString windState = citys.value(QString::fromUtf8("西安市")).value("windState").toString();
    QString windDir = citys.value(QString::fromUtf8("西安市")).value("windDir").toString();
    QString windPower = citys.value(QString::fromUtf8("西安市")).value("windPower").toString();
    int temH = citys.value(QString::fromUtf8("西安市")).value("tem2").toString().toInt();
    int temL = citys.value(QString::fromUtf8("西安市")).value("tem1").toString().toInt();
    if(temH < temL){
        int t = temL;
        temL = temH;
        temH = t;
    }
    QString tmp = QString("西安市 ");
    tmp.append(stateDetailed.toUtf8()).append("，最高温度 ")
            .append(getSpeachNum(temH))
            .append("度,最低温度 ")
            .append(getSpeachNum(temL))
            .append("度，当前温度 ")
            .append(getSpeachNum(citys.value(QString::fromUtf8("西安市")).value("temNow").toString().toInt()))
            .append("度，")
            .append(windState.toUtf8())
            .append(", ")
            .append(windDir.toUtf8())
            .append(windPower.toUtf8())
            .append(", 湿度 百分之")
            .append(getSpeachNum(humidity.left(humidity.size()-1).toInt()));
    return tmp;
}

QString Weather::getSpeachNum(int num)
{
    QString ret;
    //范围为 -99 ～ 99
    if(num > 0){
        if(num>10){
            if(num>19){
                ret.append(QString::number(num/10)).append("十");
                if(num%10 != 0){
                    ret.append(QString::number(num%10));
                }
            }else{
                ret.append("十");
                if(num%10 != 0){
                    ret.append(QString::number(num%10));
                }
            }
        }else if(num < 10){
            ret = QString(num);
        }else{
            ret = QString("十");
        }
    }else if(num < 0){
        if(num < -10){
            if(num < -19){
               ret.append("零下").append(QString::number(num/-10)).append("十");
               if(num%10 != 0){
                   ret.append(QString::number(num%10));
               }
            }else{
                ret.append("零下").append("十");
                if(num%10 != 0){
                    ret.append(QString::number(num%10));
                }
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

void Weather::speakWeather()
{
    SpeakThread* thread = new SpeakThread(getWeatherString());
    connect(thread,SIGNAL(finished()),thread,SLOT(deleteLater()));
    thread->start();
}

void Weather::replyFinished(QNetworkReply *reply)
{
    QXmlStreamReader reader;
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

    emit dataUpdate();                  //发送天气更新信号
    reply->deleteLater();
}



