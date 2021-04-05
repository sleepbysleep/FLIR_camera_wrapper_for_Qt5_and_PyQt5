#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QElapsedTimer>

#include "flir_camera.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void onReceivedImage(cv::Mat*, int, char const*);
    void onChangedExposure(int);
    void onChangedGain(int);

private:
    Ui::MainWindow *ui;
    FLIRVision::Camera* visionCamera;
    //QStatusBar* statusBar;
    QElapsedTimer elapsedTime;

};
#endif // MAINWINDOW_H
