#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <cstdlib>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <QMessageBox>

using namespace tbb;
using namespace cv;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    num = 0;

    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(clickCalibButton()));
    connect(ui->ReloadButton, SIGNAL(clicked()), this, SLOT(clickReloadButton()));
    connect(&theTimer, &QTimer::timeout, this, &MainWindow::updateImage);
    if(sceneCap.open(1) && eyeCap.open(0))
    {
        sceneImage= Mat::zeros(sceneCap.get(CV_CAP_PROP_FRAME_HEIGHT), sceneCap.get(CV_CAP_PROP_FRAME_WIDTH), CV_8UC3);
        eyeImage = Mat::zeros(eyeCap.get(CV_CAP_PROP_FRAME_HEIGHT), eyeCap.get(CV_CAP_PROP_FRAME_WIDTH), CV_8UC3);
        theTimer.start(5);
    }
    else{
        //exit(0);
    }
    sceneLabel = new QLabel(this);
    eyeLabel = new QLabel(this);
    ui->horizontalLayout->addWidget(sceneLabel);
    ui->horizontalLayout->addWidget(eyeLabel);
}

MainWindow::~MainWindow()
{
    delete sceneLabel;
    delete eyeLabel;
    delete ui;
}


void MainWindow::paintEvent(QPaintEvent *e)
{
    QImage image = QImage((uchar*)(sceneImage.data),sceneImage.cols,sceneImage.rows,QImage::Format_RGB888);
    QImage image1 = QImage((uchar*)(eyeImage.data),eyeImage.cols,eyeImage.rows,QImage::Format_RGB888);
    sceneLabel->setPixmap(QPixmap::fromImage(image));
    sceneLabel->resize(image.size());
    sceneLabel->show();
    eyeLabel->setPixmap(QPixmap::fromImage(image1));
    eyeLabel->resize(image1.size());
    eyeLabel->show();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_D)
    {
        num = num % 9;
        char eyeName[128],sceneName[128];
        sprintf((char*)&eyeName[0],"..//EyeAiming//calibImage//eye_%d.png",num);
        sprintf((char*)&sceneName[0],"..//EyeAiming//calibImage//scene_%d.png",num);
        cvtColor(sceneImage,sceneImage,CV_RGB2BGR);
        //cvtColor(eyeImage,eyeImage,CV_RGB2BGR);
        cv::imwrite(eyeName,orig_eye);
        cv::imwrite(sceneName,sceneImage);
        num++;
        this->update();
    }
    QWidget::keyPressEvent(event);
}

void MainWindow::updateImage()
{
    sceneCap>>sceneImage;
    eyeCap>>eyeImage;
    if(sceneImage.data && eyeImage.data)
    {
        orig_eye = eyeImage.clone();
        flip(sceneImage,sceneImage,0);
        Mat proc;
        cv::resize(eyeImage, proc, Size(0, 0), 0.2, 0.2, 1);
        TrackerParams params;
        params.Radius_Max = 10;
        params.Radius_Min = 8;
        params.Pupil_center = Point2f(0, 0);
        findPupilEllipse(proc, params);
        CCircle_detector::compute_ellipse(eyeImage, in, params.Pupil_center);
        eyeVector.x = params.Pupil_center.x - 320;
        eyeVector.y = params.Pupil_center.y - 240;

        gaze = Homography_map_point(eyeVector,map_matrix);
        if(IsInRightPosition(gaze,lastGaze)){
            gaze.x = lastGaze.x*0.80+gaze.x*0.20;
            gaze.y = lastGaze.y*0.80+gaze.y*0.20;
        }else{
            lastGaze.x = gaze.x;
            lastGaze.y = gaze.y;
        }
        cv::circle(sceneImage,gaze,5,CV_RGB(0,255,0),-1);
        cvtColor(sceneImage,sceneImage,CV_BGR2RGB);
        cvtColor(eyeImage,eyeImage,CV_BGR2RGB);
        this->update();
    }
}

void MainWindow::clickCalibButton()
{
    ShowCal *calshow = new ShowCal();
    calshow->show();
}

void MainWindow::clickReloadButton()
{
    FILE* fp ;
    fp = fopen("..//EyeAiming//matrix.txt", "r" );
    if(fp==NULL)
    {
        QMessageBox::information(this,"ÌבÊ¾","can not open matrix.txt!");
    }
    //     fseek( fp, 0L, SEEK_SET );
    double temp[3][3]={0};

    for(int i=0; i<3;i++)
    {

        fscanf( fp, "%lf %lf %lf", &temp[i][0],&temp[i][1],&temp[i][2]);

        this->map_matrix[i][0]=temp[i][0];
        this->map_matrix[i][1]=temp[i][1];
        this->map_matrix[i][2]=temp[i][2];
    }

    fclose(fp);
}

CvPoint2D32f  MainWindow::Homography_map_point(CvPoint2D32f  p,double map_matrix[][3])
{
    CvPoint2D32f  p2;
    double z = map_matrix[2][0]*p.x + map_matrix[2][1]*p.y + map_matrix[2][2];
    p2.x = (float)((map_matrix[0][0]*p.x + map_matrix[0][1]*p.y + map_matrix[0][2])/z);
    p2.y = (float)((map_matrix[1][0]*p.x + map_matrix[1][1]*p.y + map_matrix[1][2])/z);
    return p2;
}

bool MainWindow::IsInRightPosition(CvPoint2D32f gaze, CvPoint2D32f lastGaze){
    bool flag = false;
    if(!((lastGaze.x - gaze.x)*(lastGaze.x - gaze.x) + (lastGaze.y - gaze.y)*(lastGaze.y - gaze.y) > 30*30)){
        flag = true;
    }
    return flag;
}

