#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPaintEvent>
#include <QTimer>
#include <QPainter>
#include <QPixmap>
#include <QLabel>
#include <QImage>
#include <opencv.hpp>
#include <ui_mainwindow.h>
#include "showcal.h"
#include"PupilTracker.h"
#include"CCircle_detector.h"


using namespace cv;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    CvPoint2D32f  Homography_map_point(CvPoint2D32f  p,double map_matrix[][3]);
    bool IsInRightPosition(CvPoint2D32f gaze, CvPoint2D32f lastGaze);


public slots:
    void updateImage();
    void clickCalibButton();
    void clickReloadButton();

private:

    int num;                //number of saved images for caliberation
    QTimer theTimer;        // timer for main loop
    Mat sceneImage, eyeImage;//stand for scene and eye
    Mat orig_eye;
    VideoCapture sceneCap, eyeCap;//capture pictures
    QLabel *sceneLabel;     //point to scene label child widget
    QLabel *eyeLabel;
    double in[5];
    double     map_matrix[3][3] = {{5.340092, -1.893519, -458.326169},
                                   {1.324743, -4.774534, -378.113115},
                                   {0.003026, -0.001204, -0.820234}};
    CvPoint2D32f eyeVector;
    CvPoint2D32f lastGaze=Point2f(320.0,240.0);
    CvPoint2D32f gaze;

    Ui::MainWindow *ui;
protected:
    void paintEvent(QPaintEvent *e);
    void keyPressEvent(QKeyEvent *event);
};

#endif // MAINWINDOW_H
