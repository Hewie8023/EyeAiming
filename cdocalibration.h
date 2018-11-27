#pragma once
#ifndef CDOCALIBRATION_H
#define CDOCALIBRATION_H
#include <cv.h>
#include <highgui.h>
#include "svd.h"
#define CALIBRATIONPOINTS 9
#include "util.h"

class CDoCalibration
{
public:
    CDoCalibration();
    static	double map_matrix[3][3];
    virtual ~CDoCalibration();
    void affine_matrix_inverse(double a[][3], double r[][3]);
    void matrix_multiply33(double a[][3], double b[][3], double r[][3]);
    void cal_calibration_homography(CvPoint scenecalipoints[],CvPoint2D32f vectors[]);
    stuDPoint* normalize_point_set(stuDPoint* point_set, double &dis_scale,
                                   stuDPoint &nor_center, int num);
};

#endif // CDOCALIBRATION_H
