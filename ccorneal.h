#ifndef CCORNEAL_H
#define CCORNEAL_H
#pragma once


#define UINT8 unsigned char
#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <math.h>

#ifndef PI
#define PI 3.141592653589

#endif

class CCorneal

{
public:
    int window_size;
    CCorneal();//¹¹Ôìº¯Êý
    void remove_corneal_reflection(IplImage *image, IplImage *threshold_image, int sx, int sy,
    int biggest_crr, int &crx, int &cry, int& crr);

    int array_len;
    CvPoint corneal_reflection;
    int corneal_reflection_r;

private:
    static	double sin_array [360];
    static  double cos_array [360];

    void locate_corneal_reflection(IplImage *image, IplImage *threshold_image, int sx, int sy,
         int biggest_crar, int &crx, int &cry, int &crar);

    int fit_circle_radius_to_corneal_reflection(IplImage *image, int cx, int cy, int crar, int biggest_crar);

    void interpolate_corneal_reflection(IplImage *image, int cx, int cy, int crr);
    void SetSinCosArray();
};


#endif // CCORNEAL_H
