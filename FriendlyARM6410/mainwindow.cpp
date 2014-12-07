#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "leds.h"
#include "buttons.h"

#include <time.h>
#include <QTextCodec>
#include <QDebug>
#include <QTimer>
//#include <QTime>          //下面两个和 <rfb/rfb.h>同时存在会报错 ？
//#include <QDateTime>
#include <QStyleFactory>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

}

MainWindow::MainWindow(int argc, char** argv, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    this->argc = argc;  //maybe used
    this->argv = argv;
    ui->setupUi(this);

    wday[0]="Sun";wday[1]="Mon";wday[2]="Tue";
    wday[3]="Wed";wday[4]="Thu";wday[5]="Fri";wday[6]="Sat";

    weather = new Weather(this);
    connect(weather,SIGNAL(dataUpdate()),this,SLOT(weatherUpdate()));

    //定时器，每隔一段时间更新天气信息
    QTimer* updateWeatherTimer = new QTimer(this);
    connect(updateWeatherTimer,SIGNAL(timeout()),this,SLOT(getCityWeather()));
    updateWeatherTimer->start(1200000);
    this->getCityWeather();

    //更新时间
    QTimer* timeTimer = new QTimer(this);
    connect(timeTimer,SIGNAL(timeout()),this,SLOT(updateTime()));
    timeTimer->start(1000);

    //界面显示
    this->setWindowState(Qt::WindowFullScreen);
    this->setWindowTitle("xcorpio");
    ui->comboBox->insertItems(0,QStyleFactory::keys());
    ui->comboBox->setCurrentIndex(ui->comboBox->count()-1); //设置为最后一个样式

    //按键
    Buttons* button = new Buttons(this);
    connect(button,SIGNAL(buttonsClicked(bool*)),this,SLOT(dealButtons(bool*)));

    //蜂鸣器
    pwm = new PWM(this);

    //VNCServer
    vncServer = new VNCServer(this->argc,this->argv,this);

    //语音识别
    srProcess = new QProcess(this);
    srProcess->setReadChannel(QProcess::StandardOutput);
    connect(srProcess,SIGNAL(readyReadStandardOutput()),this,SLOT(parseSpeech()));
    connect(srProcess,SIGNAL(error(QProcess::ProcessError)),this,SLOT(processError(QProcess::ProcessError)));

    //远程摄像头
    rCamera = new RemoteCamera(this);
}

MainWindow::~MainWindow()
{
    delete ui;
    if(vncServer){
        rfbShutdownServer(vncServer->server,true);
        vncServer->quit();
        vncServer->wait(1000);
    }
    if(srProcess->state() != QProcess::NotRunning){
        srProcess->close();
        srProcess->waitForFinished(1000);
    }
    rCamera->closeCamera();
    rCamera->wait(1000);
}

void MainWindow::on_checkBox_led1_stateChanged(int arg1)
{
    if(arg1 == Qt::Unchecked){
        Leds::setLedState(0,0);
    }else if(arg1 == Qt::Checked){
        Leds::setLedState(0,1);
    }
}

void MainWindow::on_checkBox_led2_stateChanged(int arg1)
{
    if(arg1 == Qt::Unchecked){
        Leds::setLedState(1,0);
    }else if(arg1 == Qt::Checked){
        Leds::setLedState(1,1);
    }
}

void MainWindow::on_checkBox_led3_stateChanged(int arg1)
{
    if(arg1 == Qt::Unchecked){
        Leds::setLedState(2,0);
    }else if(arg1 == Qt::Checked){
        Leds::setLedState(2,1);
    }
}

void MainWindow::on_checkBox_led4_stateChanged(int arg1)
{
    if(arg1 == Qt::Unchecked){
        Leds::setLedState(3,0);
    }else if(arg1 == Qt::Checked){
        Leds::setLedState(3,1);
    }
}

void MainWindow::on_pushButton_clicked()
{
    ::system("kill -s STOP `pidof led-player`");
}

void MainWindow::on_pushButton_3_clicked()
{
    qDebug()<<QTextCodec::codecForUtfText("speak -vzh '今天是晴天！'")->name();
    ::system("speak -vzh '今天是晴天！'");
}

void MainWindow::on_pushButton_4_clicked()
{
    weather->speakWeather();
}

void MainWindow::on_pushButton_5_clicked()
{
    qDebug()<<QTextCodec::codecForLocale()->name();
    qDebug()<<QTextCodec::availableCodecs();
}

//天气信息更新，更新界面显示
void MainWindow::weatherUpdate()
{
    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);
    int hour = p->tm_hour;
    //int hour = QTime::currentTime().hour();
    QString day;
    if(hour < 18){
        day = "day";
    }else{
        day = "night";
    }
    QString weatherIconFile = "images/icon/"+day+"/"+weather->citys.value(QString::fromUtf8("西安市")).value("state1").toString()+".png";
    qDebug()<<weatherIconFile;
    ui->labelWeatherIcon->setPixmap(QPixmap(weatherIconFile));
    ui->label_humidity->setText(weather->citys.value(QString::fromUtf8("西安市")).value("humidity").toString());
    ui->label_statedetail->setText(weather->citys.value(QString::fromUtf8("西安市")).value("stateDetailed").toString());
    int temlow = weather->citys.value(QString::fromUtf8("西安市")).value("tem1").toString().toInt();
    int temHigh = weather->citys.value(QString::fromUtf8("西安市")).value("tem2").toString().toInt();
    if(temlow > temHigh){
        int tmp = temHigh;
        temHigh = temlow;
        temHigh = tmp;
    }
    ui->label_tmphigh->setText(QString::number(temHigh));
    ui->label_temlow->setText(QString::number(temlow));
    ui->label_time->setText(weather->citys.value(QString::fromUtf8("西安市")).value("time").toString());
    ui->label_tmpnow->setText(weather->citys.value(QString::fromUtf8("西安市")).value("temNow").toString());
    ui->label_windstate->setText(weather->citys.value(QString::fromUtf8("西安市")).value("windState").toString());
}

void MainWindow::getCityWeather()
{
    weather->getCityWeather("xian");
}

void MainWindow::updateTime()
{
    time_t timep;
    struct tm *p;
    time(&timep);
    p = localtime(&timep);
    ui->label_curtime->setText(QString("%1:%2:%3").arg(p->tm_hour).arg(p->tm_min).arg(p->tm_sec));
    ui->label_date->setText(QString("%1-%2-%3 %4").arg(p->tm_year+1900).arg(p->tm_mon+1).arg(p->tm_mday).arg(wday[p->tm_wday]));
    //ui->label_curtime->setText(QTime::currentTime().toString());
    //ui->label_date->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
}

void MainWindow::dealButtons(bool *on)
{
    ui->checkBox_1->setChecked(on[0]);
    ui->checkBox_2->setChecked(on[1]);
    ui->checkBox_3->setChecked(on[2]);
    ui->checkBox_4->setChecked(on[3]);
    ui->checkBox_5->setChecked(on[4]);
    ui->checkBox_6->setChecked(on[5]);
    ui->checkBox_7->setChecked(on[6]);
    ui->checkBox_8->setChecked(on[7]);
    if(on[0]){//按键1按下

    }
    if(on[1]){//按键2按下

    }
    if(on[2]){//按键3按下

    }
    if(on[3]){//按键4按下

    }
    if(on[4]){//按键5按下

    }
    if(on[5]){//按键6按下

    }
    if(on[6]){//按键7按下

    }
    if(on[7]){//按键8按下

    }
}

void MainWindow::parseSpeech()
{
    char buf[1024];
    int ret;
    while((ret = srProcess->readLine(buf, sizeof(buf))) != -1 && ret != 0) {
        QString tmp(buf);
        if(tmp.startsWith("sentence1: <s>")){
            tmp = tmp.section(" <s> ",1,1).section(" </s>",0,0);
            qDebug()<<""<<tmp;
            if(QString::compare(tmp,"TURN LIGHT ON",Qt::CaseInsensitive) == 0){
                qDebug()<<"command : "<<"TURN LIGHT ON";
                Leds::turnAllOn();
            }else if(QString::compare(tmp,"TURN LIGHT OFF",Qt::CaseInsensitive) == 0){
                qDebug()<<"command : "<<"TURN LIGHT OFF";
                Leds::turnAllOff();
            }else if(QString::compare(tmp,"REPORT WEATHER",Qt::CaseInsensitive) == 0){
                qDebug()<<"command : "<<"REPORT WEATHER";
                weather->speakWeather();
            }else if(QString::compare(tmp,"PLAY MUSIC",Qt::CaseInsensitive) == 0){
                qDebug()<<"command : "<<"PLAY MUSIC";

            }
        }
    }
}

void MainWindow::processError(QProcess::ProcessError error)
{
    switch (error) {
    case QProcess::FailedToStart:
        qDebug()<<"QProcess::FailedToStart";
        break;
    case QProcess::Crashed:
        qDebug()<<"QProcess::Crashed";
        break;
    case QProcess::Timedout:
        qDebug()<<"QProcess::Timedout";
        break;
    case QProcess::WriteError:
        qDebug()<<"QProcess::WriteError";
        break;
    case QProcess::ReadError:
        qDebug()<<"QProcess::ReadError";
        break;
    default:
        qDebug()<<"QProcess::UnknownError";
        break;
    }
}

void MainWindow::on_pushButton_6_clicked()
{
    exit(0);
}

void MainWindow::on_comboBox_currentIndexChanged(const QString &style)
{
    qApp->setStyle(QStyleFactory::create(style));
}

void MainWindow::on_pushButton_pwm_clicked()
{
    if(pwm == NULL){
        return;
    }
    if(ui->pushButton_pwm->isChecked()){
        ui->pushButton_pwm->setText(QString::fromUtf8("关闭"));
        pwm->setFreq(ui->spinBox->value());
    }else{
        ui->pushButton_pwm->setText(QString::fromUtf8("打开"));
        pwm->stopPWM();
    }
}

void MainWindow::on_spinBox_valueChanged(int arg1)
{
    if(pwm == NULL) return;
    if(ui->pushButton_pwm->isChecked()){
        pwm->setFreq(arg1);
    }
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    //改变屏幕亮度
    ::system(QString("echo %1 > /dev/backlight-1wire").arg(value).toLatin1());
}

void MainWindow::on_pushButton_7_clicked()
{
    if(!vncServer->isRunning()){
        vncServer->start();
    }
}

void MainWindow::on_pushButton_8_clicked()
{
    this->vncServer->stopServer();
}

void MainWindow::on_pushButton_9_clicked()
{
    if(srProcess->state() == QProcess::NotRunning){
        QString program = "julius";
        QStringList arguments;
        arguments<<"-C"<<"friend.jconf";
        srProcess->start(program,arguments);
    }
}

void MainWindow::on_pushButton_10_clicked()
{
    if(srProcess->state() != QProcess::NotRunning){
        srProcess->close();
    }
}

void MainWindow::on_pushButton_11_clicked()
{
    rCamera->openCamera();
}

void MainWindow::on_pushButton_12_clicked()
{
    rCamera->closeCamera();
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    if(index != 3){
        rCamera->stopPreview();
    }else if(index == 3 && rCamera->isRunning()){
        rCamera->restorePreview();
    }
}
