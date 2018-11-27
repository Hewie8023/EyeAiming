#pragma once
#ifndef SHOWCAL_H
#define SHOWCAL_H

#include <QDialog>
#include <QPainter>
#include <QPixmap>
#include <QLabel>
#include <QImage>
#include "cdocalibration.h"
#include"PupilTracker.h"
#include"CCircle_detector.h"
#ifndef CALIBRATIONPOINTS
#define CALIBRATIONPOINTS 9
#endif
using namespace cv;

namespace Ui {
class ShowCal;
}

class ShowCal : public QDialog
{
    Q_OBJECT

public:
    explicit ShowCal(QWidget *parent = 0);
    ~ShowCal();

public slots:
    void updateImage();
    void clicknextButton();
    void clickcomputeButton();

private:

    int m_CalNum;
    QPoint m_point;
    CvPoint scenecalpoints[CALIBRATIONPOINTS];
    CvPoint2D32f pucalipoints[CALIBRATIONPOINTS];
    CvPoint crcalipoints[CALIBRATIONPOINTS];
    CvPoint2D32f vectors[CALIBRATIONPOINTS];
    CDoCalibration *DoCalibration;
    double in[5];

    cv::Mat sceneImage,eyeImage;
    QLabel *sceneLabel;
    QLabel *eyeLabel;
    Ui::ShowCal *ui;
protected:
    void paintEvent(QPaintEvent *e);
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // SHOWCAL_H
