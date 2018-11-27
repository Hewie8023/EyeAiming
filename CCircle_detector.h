#pragma once

#ifndef _CCIRCLE_DECTECTOR_H
#define _CCIRCLE_DECTECTOR_H

#define UINT8 unsigned char
#include"cv.h"
#include"highgui.h"
#include <vector>
#include <stdlib.h>
//#include <stdio.h>
#include <math.h>
#include "svd.h"
#include<stdlib.h>
#include<time.h>
#include "util.h"

#ifndef PI
#define PI 3.141592653589
#endif

#ifndef MIN
#define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif

using namespace std;



class CCircle_detector
{
public:
	int edge_threshold;	//threshold of edge points detection
	int rays;			//number of rays to use to detect feature points
	int min_feature_candidates;//minimum number of circle feature candidates
	int PupilWindowSize ;  //窗口大小401
	//int height_window;	//the height of search region
	//int width_window;	//the width of searth region
	CCircle_detector();
	~CCircle_detector();
	vector<stuDPoint*> edge_point;
	vector<int> edge_intensity_diff;
	double circle_param[5];//the parameter of final circle;
	stuDPoint* normalize_point_set(stuDPoint* point_set, double &dis_scale, stuDPoint &nor_center, int num);

private:
	void get_5_random_num(int max_num, int* rand_num);
	bool solve_ellipse(double* conic_param, double* ellipse_param);
	stuDPoint* normalize_edge_point(double &dis_scale, stuDPoint &nor_center, int ep_num);
	void denormalize_ellipse_param(double* par, double* normailized_par, double dis_scale, stuDPoint nor_center);

	void destroy_edge_point();
	void locate_edge_points(unsigned char* image, int width, int height, double cx, double cy, int dis, double angle_step, double angle_normal, double angle_spread, int edge_thresh, int widthStep);
	stuDPoint get_edge_mean();

public:
	void starburst_circle_contour_detection(unsigned char* pupil_image, int width, int height, int edge_thresh, int N, int minimum_cadidate_features, double cx, double cy, int widthStep);//这里对edge_thresh是有操作的
	void pupil_fitting_inliers(unsigned char* pupil_image, int, int, int &return_max_inliers);

	void Draw_Cross(IplImage *image, int centerx, int centery, int x_cross_length, int y_cross_length, CvScalar color);

    static void compute_ellipse(cv::Mat &InImage, double* in, cv::Point2f &center);
};

#endif
