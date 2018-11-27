#include "cdocalibration.h"
#include <math.h>
double CDoCalibration::map_matrix[3][3]={0};
CDoCalibration::CDoCalibration()
{
    map_matrix[3][3]={0};
}

CDoCalibration::~CDoCalibration()
{

}

stuDPoint* CDoCalibration::normalize_point_set(stuDPoint* point_set, double &dis_scale,
                               stuDPoint &nor_center, int num)
{
    double sumx = 0, sumy = 0;
    double sumdis = 0,a;
    stuDPoint *edge = point_set;
    int i;
    for (i = 0; i < num; i++) {
      sumx =sumx + edge->x;
      a=sqrt(2.0);
      sumy =sumy + edge->y;
      sumdis = sumdis + sqrt((double)(edge->x*edge->x + edge->y*edge->y));
      edge++;
    }

    dis_scale = sqrt((double)2)*num/sumdis;
    nor_center.x = sumx*1.0/num;
    nor_center.y = sumy*1.0/num;
    stuDPoint *edge_point_nor = (stuDPoint*)malloc(sizeof(stuDPoint)*num);
    edge = point_set;
    for (i = 0; i < num; i++) {
      edge_point_nor[i].x = (edge->x - nor_center.x)*dis_scale;
      edge_point_nor[i].y = (edge->y - nor_center.y)*dis_scale;
      edge++;
    }
    return edge_point_nor;
}

void CDoCalibration::affine_matrix_inverse(double a[][3], double r[][3])
{
  double det22 = a[0][0]*a[1][1] - a[0][1]*a[1][0];
  r[0][0] = a[1][1]/det22;
  r[0][1] = -a[0][1]/det22;
  r[1][0] = -a[1][0]/det22;
  r[1][1] = a[0][0]/det22;

  r[2][0] = r[2][1] = 0;
  r[2][2] = 1/a[2][2];

  r[0][2] = -r[2][2] * (r[0][0]*a[0][2] + r[0][1]*a[1][2]);
  r[1][2] = -r[2][2] * (r[1][0]*a[0][2] + r[1][1]*a[1][2]);
}

void CDoCalibration::matrix_multiply33(double a[][3], double b[][3], double r[][3])
{
  int i, j;
  double result[9];
  double v = 0;
  for (j = 0; j < 3; j++)
  {
    for (i = 0; i < 3; i++)
    {
      v = a[j][0]*b[0][i];
      v += a[j][1]*b[1][i];
      v += a[j][2]*b[2][i];
      result[j*3+i] = v;
    }
  }
  for (i = 0; i < 3; i++)
  {
    r[i][0] = result[i*3];
    r[i][1] = result[i*3+1];
    r[i][2] = result[i*3+2];
  }
}

void CDoCalibration::cal_calibration_homography(CvPoint scenecalipoints[],CvPoint2D32f vectors[])
{
    int i, j;
    stuDPoint cal_scene[9], cal_eye[9];
    stuDPoint scene_center, eye_center, *eye_nor, *scene_nor;
    double dis_scale_scene, dis_scale_eye;
    for (i = 0; i < CALIBRATIONPOINTS; i++)
    {
      cal_scene[i].x = scenecalipoints[i].x;
      cal_scene[i].y = scenecalipoints[i].y;
      cal_eye[i].x = vectors[i].x;
      cal_eye[i].y = vectors[i].y;
    }

    scene_nor = normalize_point_set(cal_scene, dis_scale_scene, scene_center, CALIBRATIONPOINTS);
    eye_nor = normalize_point_set(cal_eye, dis_scale_eye, eye_center, CALIBRATIONPOINTS);

    const int homo_row=18, homo_col=9;
    double A[homo_row][homo_col];
    int M = homo_row, N = homo_col; //M is row; N is column
    double **ppa = (double**)malloc(sizeof(double*)*M);
    double **ppu = (double**)malloc(sizeof(double*)*M);
    double **ppv = (double**)malloc(sizeof(double*)*N);
    double pd[homo_col];
    for (i = 0; i < M; i++) {
      ppa[i] = A[i];
      ppu[i] = (double*)malloc(sizeof(double)*N);
    }
    for (i = 0; i < N; i++) {
      ppv[i] = (double*)malloc(sizeof(double)*N);
    }

    for (j = 0;  j< M; j++) {
      if (j%2 == 0) {
        A[j][0] = A[j][1] = A[j][2] = 0;
        A[j][3] = -eye_nor[j/2].x;
        A[j][4] = -eye_nor[j/2].y;
        A[j][5] = -1;
        A[j][6] = scene_nor[j/2].y * eye_nor[j/2].x;
        A[j][7] = scene_nor[j/2].y * eye_nor[j/2].y;
        A[j][8] = scene_nor[j/2].y;
      } else {
        A[j][0] = eye_nor[j/2].x;
        A[j][1] = eye_nor[j/2].y;
        A[j][2] = 1;
        A[j][3] = A[j][4] = A[j][5] = 0;
        A[j][6] = -scene_nor[j/2].x * eye_nor[j/2].x;
        A[j][7] = -scene_nor[j/2].x * eye_nor[j/2].y;
        A[j][8] = -scene_nor[j/2].x;
      }
    }
    svd(M, N, ppa, ppu, pd, ppv);

    int min_d_index = 0;
    for (i = 1; i < N; i++)
    {
      if (pd[i] < pd[min_d_index])
        min_d_index = i;
    }


    for (i = 0; i < N; i++)
    {
        map_matrix[i/3][i%3] = ppv[i][min_d_index];  //the column of v that corresponds to the smallest singular value,
                                                  //which is the solution of the equations
    }

    double T[3][3] = {0}, T1[3][3] = {0};

    T[0][0] = T[1][1] = dis_scale_eye;
    T[0][2] = -dis_scale_eye*eye_center.x;
    T[1][2] = -dis_scale_eye*eye_center.y;
    T[2][2] = 1;

    matrix_multiply33(map_matrix, T, map_matrix);

    T[0][0] = T[1][1] = dis_scale_scene;
    T[0][2] = -dis_scale_scene*scene_center.x;
    T[1][2] = -dis_scale_scene*scene_center.y;
    T[2][2] = 1;

    affine_matrix_inverse(T, T1);

    matrix_multiply33(T1, map_matrix, map_matrix);


//*
    for (i = 0; i < M; i++) {

      free(ppu[i]);
    }
    for (i = 0; i < N; i++)
    {
      free(ppv[i]);
    }
    free(ppu);
    free(ppv);
    free(ppa);

    free(eye_nor);
    free(scene_nor);
    //*/
}
