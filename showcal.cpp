#include "showcal.h"
#include "ui_showcal.h"

ShowCal::ShowCal(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShowCal)
{
    ui->setupUi(this);

    m_CalNum = 0;

    sceneLabel = new QLabel(this);
    eyeLabel = new QLabel(this);
    ui->horizontalLayout->addWidget(sceneLabel);
    ui->horizontalLayout->addWidget(eyeLabel);
    sceneLabel->installEventFilter(this);
    //connect(sceneLabel, SIGNAL(clicked()), this, SLOT(updateImage()));
    connect(ui->nextButton, SIGNAL(clicked()), this, SLOT(clicknextButton()));
    connect(ui->computeButton, SIGNAL(clicked()), this, SLOT(clickcomputeButton()));
}

ShowCal::~ShowCal()
{
    delete sceneLabel;
    delete eyeLabel;
    delete DoCalibration;
    delete ui;
}
void ShowCal::paintEvent(QPaintEvent *e)
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

void ShowCal::updateImage()
{

    char eyeName[128],sceneName[128];
    sprintf((char*)&eyeName[0],"..//EyeAiming//calibImage//eye_%d.png",m_CalNum);
    sprintf((char*)&sceneName[0],"..//EyeAiming//calibImage//scene_%d.png",m_CalNum);
    sceneImage = imread(sceneName);
    eyeImage = imread(eyeName);
    if(sceneImage.data && eyeImage.data)
    {
        Mat proc;
        cv::resize(eyeImage, proc, Size(0, 0), 0.2, 0.2, 1);
        TrackerParams params;
        params.Radius_Max = 10;
        params.Radius_Min = 8;
        params.Pupil_center = Point2f(0, 0);
        params.Corneal_center=Point2f(0,0);
        findPupilEllipse(proc, params);

        CCircle_detector::compute_ellipse(eyeImage, in, params.Pupil_center,params.Corneal_center);

        vectors[m_CalNum].x = params.Pupil_center.x - params.Corneal_center.x;
        vectors[m_CalNum].y = params.Pupil_center.y - params.Corneal_center.y;
        cvtColor(sceneImage,sceneImage,CV_BGR2RGB);
        cvtColor(eyeImage,eyeImage,CV_BGR2RGB);
        this->update();
    }
}

bool ShowCal::eventFilter(QObject *obj, QEvent *event)
{
    if(qobject_cast<QLabel*>(obj)==sceneLabel&&event->type() == QEvent::MouseButtonPress)
    {
        sceneLabel->setStyleSheet("background-color: rgb(0, 255, 255);");
        m_point = cursor().pos();
        m_point = sceneLabel->mapFromGlobal(m_point);
        scenecalpoints[m_CalNum].x = m_point.rx();
        scenecalpoints[m_CalNum].y = m_point.ry();

        cv::circle(sceneImage,cv::Point(m_point.rx(),m_point.ry()),5,CV_RGB(0,255,0),-1);
        //this->updateImage();
        m_CalNum++;
        m_CalNum = m_CalNum % 9;
        this->updateImage();
        return true;
    }
    else {
        return false;
        //return QMainWindow::eventFilter(obj, event);
    }
}
void ShowCal::clicknextButton()
{
    if(m_CalNum == 0)
    {
        this->updateImage();
    }
    else if(m_CalNum == 9){
        m_CalNum = 0;
        this->updateImage();
    }
}

void ShowCal::clickcomputeButton()
{
    DoCalibration = new CDoCalibration();
    DoCalibration->cal_calibration_homography(scenecalpoints,vectors);
    FILE  *stream ;
    stream = fopen("..//EyeAiming//matrix.txt", "w" );

    for(  int i=0; i<3; i++ )
    {

        fprintf( stream, "%lf", DoCalibration->map_matrix[i][0] );
        fprintf( stream, "%s", " ");
        fprintf( stream, "%lf", DoCalibration->map_matrix[i][1] );
        fprintf( stream, "%s", " ");
        fprintf( stream, "%lf", DoCalibration->map_matrix[i][2] );
        fprintf( stream, "%s", "\n" );

    }
    fclose(stream);
}
