#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSlider>
#include <QPixmap>

#include <iostream>
#include <exception>
#include <string>
#include <cmath>

#include "flir_camera.h"
#include <opencv2/opencv.hpp>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , visionCamera(nullptr)
{
    ui->setupUi(this);
    ui->imageCanvas->setStyleSheet("QLabel { background-color : black; color : blue; }");
    //ui->verticalLayout->addWidget(this->statusBar);
    statusBar()->showMessage("Hello world.", 3000);

    try {
        FLIRVision::initSystem();
        this->visionCamera = new FLIRVision::Camera(/*"serialNumber"*/);

        std::string window_title = "Qt Camera - " + this->visionCamera->getDeviceName();
        this->setWindowTitle(QString::fromStdString(window_title));
        QObject::connect(this->visionCamera, &FLIRVision::Camera::imageArrived, this, &MainWindow::onReceivedImage);

        int minValue = (int)std::floor(this->visionCamera->getMinExposure());
        //int maxValue = (int)std::ceil(this->visionCamera->getMaxExposure());
        int maxValue = 100000;
        int value = (int)std::round(this->visionCamera->getExposure());
        ui->exposureSlider->setRange(minValue, maxValue);
        ui->exposureSlider->setValue(value);
        ui->exposureLabel->setText(QString::number(value));
        QObject::connect(ui->exposureSlider, &QSlider::valueChanged, this, &MainWindow::onChangedExposure);

        minValue = (int)std::floor(this->visionCamera->getMinGain());
        maxValue = (int)std::ceil(this->visionCamera->getMaxGain());
        value = (int)std::round(this->visionCamera->getGain());
        ui->gainSlider->setRange(minValue, maxValue);
        ui->gainSlider->setValue(value);
        ui->gainLabel->setText(QString::number(value));
        QObject::connect(ui->gainSlider, &QSlider::valueChanged, this, &MainWindow::onChangedGain);

        this->visionCamera->enableAcquisition();
        this->elapsedTime.start();
    } catch (std::exception& e) {
        std::cerr << __FILE__ << ":" << __LINE__ << "> " << e.what() << std::endl;
    }
}

MainWindow::~MainWindow()
{
    try {
        if (this->visionCamera) delete this->visionCamera;
        FLIRVision::deinitSystem();
    } catch (std::exception& e) {
        std::cerr << __FILE__ << ":" << __LINE__ << "> " << e.what() << std::endl;
    }
    delete ui;
}

void MainWindow::onReceivedImage(cv::Mat* image, int count, char const* serial)
{
    static int fps_count = 0;
    static double fps_sum = 0;

    //std::cout << count << ", " << serial << "\n";
    //std::cout << ui->imageCanvas->size().width() << ", " << ui->imageCanvas->size().height() << "\n";
    QImage qimage = QImage(image->data, image->cols, image->rows, image->step, QImage::Format_RGB888);//.rgbSwapped();
    ui->imageCanvas->setPixmap(QPixmap::fromImage(qimage.scaled(ui->imageCanvas->size(),
                                                                Qt::KeepAspectRatio,
                                                                Qt::SmoothTransformation)));
    delete image;

    int elapsed = this->elapsedTime.elapsed();
    fps_sum += 1000.0/(double)elapsed;
    ++fps_count;
    if (fps_count >= 100) {
        statusBar()->showMessage(QString("%1 [fps]").arg(fps_sum/fps_count, 3, 'f', 0, ' '));
        fps_sum = 0.0;
        fps_count = 0;
    }
    this->elapsedTime.restart();
}

void MainWindow::onChangedExposure(int value)
{
    ui->exposureSlider->setValue(value);
    ui->exposureLabel->setText(QString("%1").arg(value, 8, 'd', 0, '0'));
    try {
        this->visionCamera->setExposure(value);
    } catch (std::exception& e) {
        std::cerr << __FILE__ << ":" << __LINE__ << ">" << e.what() << "\n";
    }
}

void MainWindow::onChangedGain(int value)
{
    ui->gainSlider->setValue(value);
    ui->gainLabel->setText(QString("%1").arg(value, 8, 'd', 0, '0'));
    try {
        this->visionCamera->setGain(value);
    } catch (std::exception& e) {
        std::cerr << __FILE__ << ":" << __LINE__ << ">" << e.what() << "\n";
    }
}
