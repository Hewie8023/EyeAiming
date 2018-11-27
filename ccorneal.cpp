#include "ccorneal.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


double CCorneal::sin_array[360]={0};
double CCorneal::cos_array[360]={0};

//初始化类的静态成员变量


void CCorneal::SetSinCosArray()
{
    for (int i = 0; i < 360; i++)
    {
     sin_array[i] = sin(i*PI/180);
     cos_array[i] = cos(i*PI/180);
    }
}

CCorneal::CCorneal()
{

    corneal_reflection.x=0;
    corneal_reflection.y=0;
    corneal_reflection_r=0;
    window_size=251;//窗口的大小默认为201;
    array_len=360;

}

void CCorneal::remove_corneal_reflection(IplImage *image, IplImage *threshold_image, int sx, int sy, int biggest_crr,
                                         int& crx, int& cry, int& crr)
{

    int crar = -1;	//corneal reflection approximate radius
  crx = cry = crar = -1;
  SetSinCosArray();

  locate_corneal_reflection(image, threshold_image, sx, sy, (int)(biggest_crr/2.5), crx, cry, crar);
  crr = fit_circle_radius_to_corneal_reflection(image, crx, cry, crar, (int)(biggest_crr/2.5));
  crr = (int)(2.5*crr);
  interpolate_corneal_reflection(image, crx, cry, crr);


}

void CCorneal::locate_corneal_reflection(IplImage *image, IplImage *threshold_image, int sx, int sy, int biggest_crar, int &crx, int &cry, int &crar)
{
 //窗口必须是奇数
    if (window_size%2 == 0) {
    printf("Error! window_size should be odd!\n");
  }

  int r = (window_size-1)/2;
  int startx = MAX(sx-r, 0);
  int endx = MIN(sx+r, image->width-1);
  int starty = MAX(sy-r, 0);
  int endy = MIN(sy+r, image->height-1);

  //缩小搜索区域
  cvSetImageROI(image, cvRect(startx, starty, endx-startx+1, endy-starty+1));
  cvSetImageROI(threshold_image, cvRect(startx, starty, endx-startx+1, endy-starty+1));

  double min_value, max_value;
  CvPoint min_loc, max_loc; //location
  cvMinMaxLoc(image, &min_value, &max_value, &min_loc, &max_loc);//相当于belable


  //这里做相当于matlab里regionprops函数
  int threshold, i;
  CvSeq* contour=NULL;//为边界分配一个动态的序列

  CvMemStorage* storage = cvCreateMemStorage(0);//创建内存块默认64k


  double *scores = (double*)malloc(sizeof(double)*((int)max_value+1));//按照找出来的最大值设置比例个数

  memset(scores, 0, sizeof(double)*((int)max_value+1));//置零

  int area, max_area, sum_area;//设置比例常数


  for (threshold = (int)max_value; threshold >= 1; threshold--) //循环开始，threshlod从max_value开始逐步-1
  {



    //《1》  二值化图像〉=threshold都设置成max_value 剩下的变成0黑色

     cvThreshold(image, threshold_image, threshold, 1, CV_THRESH_BINARY);

    //《2》  提取当前 threshold 所有轮廓，并且放置在 list 中 ，并且将所有点由链码形式翻译(转化）为点序列形式

    cvFindContours(threshold_image, storage, &contour, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

    max_area = 0;//当前情况下最大面积
    sum_area = 0;//当前情况下总面积

    CvSeq *max_contour = contour;//当前情况下最大面积的曲线

    //《3》  寻找当前 threshold下的总面积sum_area 和最大面积max_area，和max_contour
    for( ; contour != 0; contour = contour->h_next)
    {
      area = contour->total + (int)(fabs(cvContourArea(contour, CV_WHOLE_SEQ)));//contour->total相当于周长或者最外圈的面积
      sum_area += area;
      if (area > max_area) {
        max_area = area;
        max_contour = contour;
      }
    }

    //《4》  循环完成，计算比值score
    if (sum_area-max_area > 0) // 这里是说 因为总面积sum_area = 最大面积max_area 就只有一块区域了
    {
      scores[threshold-1] = max_area / (sum_area-max_area);
      //printf("max_area: %d, max_contour: %d, sum_area: %d; scores[%d]: %lf\n",
      //        max_area, max_contour->total, sum_area, threshold-1, scores[threshold-1]);
    }
    else
      continue; //这里continue的是threshold的循环===〉跳出后面的语句

    //《5》 在当前 threshold 下 找到了面积，计算它的角膜亮斑的近似位置crx，cry和 近似半径crar
    if (scores[threshold-1] - scores[threshold] < 0) {
      //found the corneal reflection
      crar = (int)sqrt(max_area / PI);
      int sum_x = 0;
      int sum_y = 0;
      CvPoint *point;
      for (i = 0; i < max_contour->total; i++) {
        point = CV_GET_SEQ_ELEM(CvPoint, max_contour, i);
        sum_x += point->x;
        sum_y += point->y;
      }
      crx = sum_x/max_contour->total;
      cry = sum_y/max_contour->total;
      break;
    }
  }
  /*/ printf("(corneal reflection) max_value = %lf; threshold = %d\n", max_value, threshold);
  printf("(corneal reflection) Scores:\n");
  for (int i = (int)max_value; i >= threshold-1; i--) {
    printf("%6.2lf", scores[i]);
  }
  printf("\n");*/

  free(scores);
  cvReleaseMemStorage(&storage);
  cvResetImageROI(image);
  cvResetImageROI(threshold_image);

  if (crar > biggest_crar) {
    printf("(corneal) size wrong! crx:%d, cry:%d, crar:%d (should be less than %d)\n", crx, cry, crar, biggest_crar);
    cry = crx = -1;
    crar = -1;
  }

  if (crx != -1 && cry != -1) {
    printf("(corneal) startx:%d, starty:%d, crx:%d, cry:%d, crar:%d\n", startx, starty, crx, cry, crar);
    crx += startx;
    cry += starty;
  }

}

int CCorneal:: fit_circle_radius_to_corneal_reflection(IplImage *image, int crx, int cry, int crar, int biggest_crar)
{
  if (crx == -1 || cry == -1 || crar == -1)
    return -1;

  double *ratio = (double*)malloc((biggest_crar-crar+1)*sizeof(double));
  int i, r, r_delta=1;
  int x, y, x2, y2;
  double sum, sum2;
  for (r = crar; r <= biggest_crar; r++) {
    sum = 0;
    sum2 = 0;
    for (i = 0; i < array_len; i++) {
      x = (int)(crx + (r+r_delta)*cos_array[i]);
      y = (int)(cry + (r+r_delta)*sin_array[i]);
      x2 = (int)(crx + (r-r_delta)*cos_array[i]);
      y2 = (int)(cry + (r-r_delta)*sin_array[i]);                       //It appears wrong,so I change '+' to '-'; y2 = (int)(cry + (r+r_delta)*sin_array[i]);
      if ((x >= 0 && y >=0 && x < image->width && y < image->height) &&
          (x2 >= 0 && y2 >=0 && x2 < image->width && y2 < image->height)) {
        sum += *(image->imageData+y*image->width+x);
        sum2 += *(image->imageData+y2*image->width+x2);
      }
    }
    ratio[r-crar] = sum / sum2;
    if (r - crar >= 2) {
      if (ratio[r-crar-2] < ratio[r-crar-1] && ratio[r-crar] < ratio[r-crar-1]) {
        free(ratio);
        return r-1;
      }
    }
  }

  free(ratio);
  printf("ATTN! fit_circle_radius_to_corneal_reflection() do not change the radius\n");
  return crar;
}

void CCorneal:: interpolate_corneal_reflection(IplImage *image, int crx, int cry, int crr)
{
  if (crx == -1 || cry == -1 || crr == -1)
    return;

  if (crx-crr < 0 || crx+crr >= image->width || cry-crr < 0 || cry+crr >= image->height) {
    printf("Error! Corneal reflection is too near the image border\n");
    return;
  }

  int i, r, r2,  x, y;
  UINT8 *perimeter_pixel = (UINT8*)malloc(array_len*sizeof(int));
  int sum=0; //pixel_value;
  double avg;
  for (i = 0; i < array_len; i++) {
    x = (int)(crx + crr*cos_array[i]);
    y = (int)(cry + crr*sin_array[i]);
    perimeter_pixel[i] = (UINT8)(*(image->imageData+y*image->width+x));
    sum += perimeter_pixel[i];
  }
  avg = sum*1.0/array_len;

  for (r = 1; r < crr; r++) {
    r2 = crr-r;
    for (i = 0; i < array_len; i++) {
      x = (int)(crx + r*cos_array[i]);
      y = (int)(cry + r*sin_array[i]);
      *(image->imageData+y*image->width+x) = (UINT8)((r2*1.0/crr)*avg + (r*1.0/crr)*perimeter_pixel[i]);
    }
    //printf("r=%d: %d (avg:%lf, end:%d)\n", r, (UINT8)((r2*1.0/crr)*avg + (r*1.0/crr)*perimeter_pixel[i-1]),
    //       avg, perimeter_pixel[i-1]);
  }
  free(perimeter_pixel);
}
