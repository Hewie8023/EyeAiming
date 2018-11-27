
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("眼动瞄准");
    w.show();

    return a.exec();
}

/*
#include"PupilTracker.h"
#include"CCircle_detector.h"


using namespace std;
using namespace tbb;
using namespace cv;

void compute_ellipse(Mat &InImage, double* in, Point2f center)
{
    clock_t start = clock();
    char param_str[256];
    IplImage *rgbI = &IplImage(InImage);
    IplImage * image = cvCreateImage(cvGetSize(rgbI), IPL_DEPTH_8U, 1);//单通道处理
    if (InImage.channels() == 3)
    {
        cvCvtColor(rgbI, image, CV_BGR2GRAY);
    }
    else
    {
        cvCopy(rgbI, image);
    }
    cvSmooth(image, image, CV_GAUSSIAN, 5, 5);

    CCircle_detector inner_circle;
    if (center.x == 0)
    {
        inner_circle.starburst_circle_contour_detection((unsigned char*)image->imageData, image->width, image->height,
            inner_circle.edge_threshold, inner_circle.rays, inner_circle.min_feature_candidates, (double)image->width / 2, (double)image->height / 2, image->widthStep / sizeof(uchar));
    }
    else
    {
        inner_circle.starburst_circle_contour_detection((unsigned char*)image->imageData, image->width, image->height,
            inner_circle.edge_threshold, inner_circle.rays, inner_circle.min_feature_candidates, center.x, center.y, image->widthStep / sizeof(uchar));
    }

    int inliers_num = 0;
    CvSize ellipse_axis;
    if (inner_circle.edge_point.size() > 15)
    {
        inner_circle.pupil_fitting_inliers((unsigned char*)image->imageData, image->width, image->height, inliers_num);
        ellipse_axis.width = (int)inner_circle.circle_param[0];
        ellipse_axis.height = (int)inner_circle.circle_param[1];
        CvPoint pupil = { 0,0 };
        pupil.x = inner_circle.circle_param[2];
        pupil.y = inner_circle.circle_param[3];
        //cout << "inner_circle=" << pupil.x << " " << pupil.y << " " << ellipse_axis.width << " " << ellipse_axis.height << " " << inner_circle.circle_param[4] << endl;
        CvScalar Red = CV_RGB(255, 0, 0);
        cvEllipse(rgbI, pupil, ellipse_axis, -inner_circle.circle_param[4] * 180 / PI, 0, 360, Red, 1);
        inner_circle.Draw_Cross(rgbI, pupil.x, pupil.y, 5, 5, Red);
        //sprintf(param_str, "x=%.2f y=%.2f a=%.2f b=%.2f theta=%.6f", inner_circle.circle_param[2], inner_circle.circle_param[3], inner_circle.circle_param[0], inner_circle.circle_param[1], inner_circle.circle_param[4]);
        //std::string innerString("inner:");
        //innerString += param_str;
        //cv::putText(InImage, // 图像矩阵
        //	innerString,                  // string型文字内容
        //	cv::Point(10, 100),           // 文字坐标，以左下角为原点
        //	cv::FONT_HERSHEY_DUPLEX,   // 字体类型
        //	0.5, // 字体大小
        //	cv::Scalar(0, 0, 255),
        //	3, 8, 0);       // 字体颜色
        for (int i = 0; i < 5; i++)
        {
            in[i] = inner_circle.circle_param[i];
        }
    }
    //std::cout << double(clock() - start) / CLOCKS_PER_SEC << std::endl;
    //cvNamedWindow("Image:", WINDOW_NORMAL);
    //cvMoveWindow("Image:", 100, 100);
    //cvShowImage("Image:", rgbI);
    //cvWaitKey(0);
    //cvDestroyWindow("Image:");

    rgbI = NULL;
    cvReleaseImage(&image);
}
int main()
{

    Mat image = imread("D:\\Cal\\Eye_0007.jpg",1);//"C:\\Users\\mby\\Desktop\\QQ截图20181121192125.jpg"
    Mat proc;
    double in[5];
    VideoCapture cap(0);
    cvNamedWindow("Image:", WINDOW_AUTOSIZE);//WINDOW_NORMAL
    //cvMoveWindow("Image:", 100, 100);
    while (1)
    {
        if (cap.isOpened())
        {
            clock_t start = clock();
            cap >> image;
            resize(image, proc, Size(0, 0), 0.2, 0.2, 1);
            TrackerParams params;
            params.Radius_Max = 10;
            params.Radius_Min = 8;
            params.Pupil_center = Point2f(0, 0);


            findPupilEllipse(proc, params);
            compute_ellipse(image, in, params.Pupil_center);



            imshow("Image:", image);

            char c = cv::waitKey(5); //延时30毫秒
            std::cout << double(clock() - start) / CLOCKS_PER_SEC << std::endl;

            if (c == 27)
            {
                break;
            }
        }
        else
        {
            break;
        }
    }



    cvDestroyWindow("Image:");
    return 0;

}
*/
