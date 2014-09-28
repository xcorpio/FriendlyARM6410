#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "leds.h"
#include "stdlib.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
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
    ::system("speak -vzh '今天是晴天！'");
}
