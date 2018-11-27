#pragma once
#ifndef __PUPILTRACKER_H__
#define __PUPILTRACKER_H__

#include <tbb/tbb.h>
#include<iostream>
#include <time.h>
#include<cv.hpp>
#include<opencv\highgui.h>
#include<opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>


const double SQRT_2 = std::sqrt(2.0);
const double PI = CV_PI;
const cv::Point2f UNKNOWN_POSITION = cv::Point2f(-1, -1);
struct TrackerParams
{
	int Radius_Min;
	int Radius_Max;
	cv::Point2f Pupil_center;
    cv::Point2f Corneal_center;
};

bool findPupilEllipse(const cv::Mat &m,TrackerParams &params);









#endif
