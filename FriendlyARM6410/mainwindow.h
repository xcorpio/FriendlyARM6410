#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "weather.h"
#include "pwm.h"
#include "vncserver.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    MainWindow(int argc,char** argv,QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_checkBox_led1_stateChanged(int arg1);

    void on_checkBox_led2_stateChanged(int arg1);

    void on_checkBox_led3_stateChanged(int arg1);

    void on_checkBox_led4_stateChanged(int arg1);

    void on_pushButton_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_comboBox_currentIndexChanged(const QString &style);

    void on_pushButton_pwm_clicked();

    void on_spinBox_valueChanged(int arg1);


    void on_horizontalSlider_valueChanged(int value);

    void on_pushButton_7_clicked();

public slots:
    void weatherUpdate();
    void getCityWeather();
    void updateTime();
    void dealButtons(bool* on);

private:
    Ui::MainWindow *ui;
    int argc;
    char** argv;
    QString wday[7];
    Weather * weather;
    PWM* pwm;
    VNCServer* vncServer;
};

#endif // MAINWINDOW_H
