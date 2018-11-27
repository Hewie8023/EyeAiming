#include "CCircle_detector.h"
#include "ccorneal.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

CCircle_detector::CCircle_detector()
{
    edge_threshold = 50;		//threshold of pupil edge points detection
    rays = 30;				//number of rays to use to detect feature points
    min_feature_candidates = 20;
    PupilWindowSize = 201;  //我自己添加的窗口大小401
	//height_window = 401;  
	//width_window = 401;
	for (int i = 0; i < 5; i++)
		circle_param[i] = 0;
}
CCircle_detector::~CCircle_detector()
{

}
void CCircle_detector::starburst_circle_contour_detection(unsigned char* pupil_image, int width, int height, int edge_thresh, int N, int minimum_cadidate_features, double cx, double cy, int widthStep)
{

	int dis = 3; //步长
	double angle_spread = 100 * PI / 180;//散射角度
	int loop_count = 0;
	double angle_step = 2 * PI / N;
	double new_angle_step;
	stuDPoint *edge, edge_mean;
	double angle_normal;//起始点

	int first_ep_num;

	while (edge_thresh > 5 && loop_count <= 10) //循环是控制当前找到的点是否收敛
	{
		edge_intensity_diff.clear();//清空操作
		destroy_edge_point();
		//edge_point在头文件里定义，是stuDPoint结构
		//minimum_cadidate_features头文件定义10个

		while (edge_point.size() < minimum_cadidate_features && edge_thresh > 5) //如果特征点的个数不够要求
		{

			//《1。1》如果特征点的个数不够要求，现清空原来的纪录
			edge_intensity_diff.clear();
			destroy_edge_point();


			//《1。2》清空原来的纪录以后，开始找点, double angle_step = 2*PI/N;
			locate_edge_points(pupil_image, width, height, cx, cy, dis, angle_step, 0, 2 * PI, edge_thresh, widthStep);
			//《1。3》如果特征点的个数不够要求，改变阈值直道edge_thresh=5
			if (edge_point.size() < minimum_cadidate_features)
			{
				edge_thresh -= 1;//对符合的点的阈值要求5<edge_thresh<20
			}
		}
		if (edge_thresh <= 5)
		{
			break;//这里是从第一个while中跳出去
		}

		//如果阈值edge_thresh还是比5大，而且个数也够，就开始反着找
		first_ep_num = edge_point.size();
		for (int i = 0; i < first_ep_num; i++)
		{
			edge = edge_point.at(i);
			angle_normal = atan2(cy - edge->y, cx - edge->x);
			new_angle_step = angle_step*(edge_thresh*1.0 / edge_intensity_diff.at(i));
			locate_edge_points(pupil_image, width, height, edge->x, edge->y, dis, new_angle_step, angle_normal,
				angle_spread, edge_thresh, widthStep);
		}

		loop_count += 1;//对循环加一，

		edge_mean = get_edge_mean();//对edge_point中的点取平均edge_mean，

		if (fabs(edge_mean.x - cx) + fabs(edge_mean.y - cy) < 10)
			break;//这里是说当前点相对稳定就跳出来

		cx = edge_mean.x;
		cy = edge_mean.y;
	}

	if (loop_count > 10) {
		destroy_edge_point();//超过的话，就销毁
		//printf("Error! edge points did not converge in %d iterations!\n", loop_count);
		return;
	}

	if (edge_thresh <= 5) {
		destroy_edge_point();
		//    printf("Attention! No. %d Adaptive threshold is too low!\n",count_number);
		//printf("Attention!  Adaptive threshold is too low!\n");
		return;
	}
}

//这个函数找到从中心出发的符合阈值的点的集合（2PI），数据存储在edge_point和edge_intensity_diff
void CCircle_detector::locate_edge_points(unsigned char* image, int width, int height, double cx, double cy, int dis, double angle_step, double angle_normal, double angle_spread, int edge_thresh, int widthStep)
{
	//这里原函数调用的参数
	// locate_edge_points(pupil_image, width, height, cx, cy, dis, angle_step, 0, 2*PI, edge_thresh);
	double angle;
	stuDPoint p, *edge;
	double dis_cos, dis_sin;
	int pixel_value1, pixel_value2;

	int r = (PupilWindowSize - 1) / 2;
	int startx = MAX(cx - r, 0);
	int endx = MIN(cx + r, width - 1);
	int starty = MAX(cy - r, 0);
	int endy = MIN(cy + r, height - 1);


	for (angle = angle_normal - angle_spread / 2 + 0.0001; angle < angle_normal + angle_spread / 2; angle += angle_step)
	{
		dis_cos = dis * cos(angle);
		dis_sin = dis * sin(angle);
		p.x = cx + dis_cos;//每次加的步长
		p.y = cy + dis_sin;



		//下面的循环是在当前角度下，伸展半径找符合阈值的点
		pixel_value1 = image[(int)(p.y)*widthStep + (int)(p.x)];//取当前点的像素值
		while (1) {//在这个循环中找在当前角度下 到符合pupil_edge_thres的点
			p.x += dis_cos;
			p.y += dis_sin;
			if (p.x < startx || p.x >= endx || p.y < starty || p.y >= endy)//当前点必须在ROI范围内
																		   //		if (p.x < 0 || p.x >= width || p.y < 0 || p.y >= height)//当前点必须在ROI范围内
				break;

			pixel_value2 = image[(int)(p.y)*widthStep + (int)(p.x)];
			if (abs(pixel_value2 - pixel_value1) > edge_thresh)
			{//找到了点，放到edge_point里面，同时把当前的差值放到edge_intensity_diff
				edge = (stuDPoint*)malloc(sizeof(stuDPoint));
				edge->x = p.x - dis_cos / 2;
				edge->y = p.y - dis_sin / 2;
				edge_point.push_back(edge);
				edge_intensity_diff.push_back(abs(pixel_value2 - pixel_value1));
				break;
			}
			pixel_value1 = pixel_value2;//没找到，继续while循环，伸长半径
		}
	}
}

stuDPoint CCircle_detector::get_edge_mean()//这个函数没有传参，但是调用的是全局变量edge_point，返回edge_mean
{
	stuDPoint *edge;
	int i;
	double sumx = 0, sumy = 0;
	stuDPoint edge_mean;
	for (i = 0; i < edge_point.size(); i++) {
		edge = edge_point.at(i);
		sumx += edge->x;
		sumy += edge->y;
	}
	if (edge_point.size() != 0) {
		edge_mean.x = sumx / edge_point.size();
		edge_mean.y = sumy / edge_point.size();
	}
	else {
		edge_mean.x = -1;
		edge_mean.y = -1;
	}
	return edge_mean;
}

void CCircle_detector::destroy_edge_point()//逐点删除
{
	vector <stuDPoint*>::iterator iter;//迭带器

	if (edge_point.size() != 0) {
		for (iter = edge_point.begin(); iter != edge_point.end(); iter++) {
			free(*iter);
		}
		edge_point.clear();
	}
}


//------------ Ransac ellipse fitting -----------//
// Randomly select 5 indeics
//Random seed need set
void CCircle_detector::get_5_random_num(int max_num, int* rand_num)//max_num=ep_num-1
{
	int rand_index = 0;
	int r;
	int i;
	bool is_new = 1;

	if (max_num == 4)//若指定个数是4个，5个任意数字是0~4 
	{
		for (i = 0; i < 5; i++)
		{
			rand_num[i] = i;
		}
		return;
	}

	while (rand_index < 5)
	{
		is_new = 1;
		r = (int)((rand()*1.0 / RAND_MAX) * max_num);//rand 需要伪随机
		for (i = 0; i < rand_index; i++)
		{
			if (r == rand_num[i])
			{
				is_new = 0;
				break;
			}
		}
		if (is_new)
		{
			rand_num[rand_index] = r;
			rand_index++;
		}
	}
}


// solve_ellipse
// conic_param[6] is the parameters of a conic {a, b, c, d, e, f}; conic equation: ax^2 + bxy + cy^2 + dx + ey + f = 0;
// ellipse_param[5] is the parameters of an ellipse {ellipse_a, ellipse_b, cx, cy, theta}; a & b is the major or minor axis; 
// cx & cy is the ellipse center; theta is the ellipse orientation
bool CCircle_detector::solve_ellipse(double* conic_param, double* ellipse_param)
{
	double a = conic_param[0];
	double b = conic_param[1];
	double c = conic_param[2];
	double d = conic_param[3];
	double e = conic_param[4];
	double f = conic_param[5];
	//get ellipse orientation
	double theta = atan2(b, a - c) / 2;

	//get scaled major/minor axes
	double ct = cos(theta);
	double st = sin(theta);
	double ap = a*ct*ct + b*ct*st + c*st*st;
	double cp = a*st*st - b*ct*st + c*ct*ct;

	//get translations
	double cx = (2 * c*d - b*e) / (b*b - 4 * a*c);
	double cy = (2 * a*e - b*d) / (b*b - 4 * a*c);

	//get scale factor
	double val = a*cx*cx + b*cx*cy + c*cy*cy;
	double scale_inv = val - f;

	if (scale_inv / ap <= 0 || scale_inv / cp <= 0) {
		//printf("Error! ellipse parameters are imaginary a=sqrt(%lf), b=sqrt(%lf)\n", scale_inv/ap, scale_inv/cp);
		memset(ellipse_param, 0, sizeof(double) * 5);
		return 0;
	}

	ellipse_param[0] = sqrt(scale_inv / ap);
	ellipse_param[1] = sqrt(scale_inv / cp);
	ellipse_param[2] = cx;
	ellipse_param[3] = cy;
	ellipse_param[4] = theta;
	return 1;
}

//一致化坐标的过程，返回edge_point_nor和dis_scale
//次函数未被使用
stuDPoint* CCircle_detector::normalize_point_set(stuDPoint* point_set, double &dis_scale,
	stuDPoint &nor_center, int num)
{
	double sumx = 0, sumy = 0;
	double sumdis = 0;
	stuDPoint *edge = point_set;
	int i;
	for (i = 0; i < num; i++) {
		sumx += edge->x;
		sumy += edge->y;
		sumdis += sqrt((double)(edge->x*edge->x + edge->y*edge->y));
		edge++;
	}

	dis_scale = sqrt((double)2)*num / sumdis;
	nor_center.x = sumx*1.0 / num;
	nor_center.y = sumy*1.0 / num;
	stuDPoint *edge_point_nor = (stuDPoint*)malloc(sizeof(stuDPoint)*num);
	edge = point_set;
	for (i = 0; i < num; i++) {
		edge_point_nor[i].x = (edge->x - nor_center.x)*dis_scale;
		edge_point_nor[i].y = (edge->y - nor_center.y)*dis_scale;
		edge++;
	}
	return edge_point_nor;
}

//一致化坐标的过程，返回edge_point_nor和dis_scale
stuDPoint* CCircle_detector::normalize_edge_point(double &dis_scale, stuDPoint &nor_center, int ep_num)
//前两个参数都是传出来的,传递给pupil_fitting_inliers()
{
	double sumx = 0, sumy = 0;
	double sumdis = 0;
	stuDPoint *edge;
	int i;
	for (i = 0; i < ep_num; i++) {
		edge = edge_point.at(i);
		sumx += edge->x;
		sumy += edge->y;
		sumdis += sqrt((double)(edge->x*edge->x + edge->y*edge->y));
	}

	dis_scale = sqrt((double)2)*ep_num / sumdis;
	nor_center.x = sumx*1.0 / ep_num;
	nor_center.y = sumy*1.0 / ep_num;
	stuDPoint *edge_point_nor = (stuDPoint*)malloc(sizeof(stuDPoint)*ep_num);
	for (i = 0; i < ep_num; i++) {
		edge = edge_point.at(i);
		edge_point_nor[i].x = (edge->x - nor_center.x)*dis_scale;
		edge_point_nor[i].y = (edge->y - nor_center.y)*dis_scale;
	}
	return edge_point_nor;
}

//还原一致化坐标到xy坐标系
void CCircle_detector::denormalize_ellipse_param(double* par, double* normailized_par, double dis_scale, stuDPoint nor_center)
{
	par[0] = normailized_par[0] / dis_scale;	//major or minor axis
	par[1] = normailized_par[1] / dis_scale;
	par[2] = normailized_par[2] / dis_scale + nor_center.x;	//ellipse center
	par[3] = normailized_par[3] / dis_scale + nor_center.y;
}

void CCircle_detector::pupil_fitting_inliers(unsigned char* pupil_image, int width, int height, int &return_max_inliers_num)
//vector <stuDPoint*> & edge_point,double * pupil_param)
{
	int i;
	int ep_num = edge_point.size();   //ep stands for edge point
	stuDPoint nor_center;
	double dis_scale;

	//这里判断个数是否够5个点,否则就把pupil_param置成0

	int ellipse_point_num = 5;	//number of point that needed to fit an ellipse
	if (ep_num < ellipse_point_num)
	{
		//printf("Error! %d points are not enough to fit ellipse\n", ep_num);
		memset(circle_param, 0, sizeof(circle_param));
		return_max_inliers_num = 0;
		//   exit(0);
		return;
	}

	//把找出的点一致化edge_point_nor
	//Normalization
	stuDPoint *edge_point_nor = normalize_edge_point(dis_scale, nor_center, ep_num);


	//===================
	//  RANSAC主程序
	//===================
	//Ransac
	int *inliers_index = (int*)malloc(sizeof(int)*ep_num);//每次解SVD复合条件的内点个数，ep_num是所有点的个数
														  //因为暂时不需要用这些点，所以不用传回来
	int *max_inliers_index = (int*)malloc(sizeof(int)*ep_num);//把每次SVD做出来的最多的点存在此
	int ninliers = 0;//对应inliers_index
	int max_inliers = 0;//对应max_inliers_index
	int sample_num = 1000;	//number of sample
	int ransac_count = 0;  //当前所作的循环次数
	double dis_threshold = sqrt(3.84)*dis_scale;//满足条件的误差
	double dis_error;//实际的误差

	memset(inliers_index, int(0), sizeof(int)*ep_num); //给inliers_index初始值赋为0
	memset(max_inliers_index, int(0), sizeof(int)*ep_num);//max_inliers_index初始值赋为0
	int rand_index[5];//数组 放置任意产生的5个数
	double A[6][6];
	int M = 6, N = 6; //M is row; N is column
	for (i = 0; i < N; i++) {
		A[i][5] = 1;
		A[5][i] = 0;
	}
	double **ppa = (double**)malloc(sizeof(double*)*M);
	double **ppu = (double**)malloc(sizeof(double*)*M);
	double **ppv = (double**)malloc(sizeof(double*)*N);
	for (i = 0; i < M; i++)
	{
		ppa[i] = A[i];
		ppu[i] = (double*)malloc(sizeof(double)*N);
	}
	for (i = 0; i < N; i++)
	{
		ppv[i] = (double*)malloc(sizeof(double)*N);
	}
	double pd[6];
	int min_d_index;
	double conic_par[6] = { 0 };
	double ellipse_par[5] = { 0 };
	double best_ellipse_par[5] = { 0 };
	double ratio;


    //srand(0);//设置随机数发生器
	//大循环从这里开始
	while (sample_num > ransac_count)
	{
		get_5_random_num((ep_num - 1), rand_index);//找到5个数放到rand_index中

												   //svd decomposition to solve the ellipse parameter
		for (i = 0; i < 5; i++)
		{
			A[i][0] = edge_point_nor[rand_index[i]].x * edge_point_nor[rand_index[i]].x;//x^x
			A[i][1] = edge_point_nor[rand_index[i]].x * edge_point_nor[rand_index[i]].y;//x^y
			A[i][2] = edge_point_nor[rand_index[i]].y * edge_point_nor[rand_index[i]].y;//y^y
			A[i][3] = edge_point_nor[rand_index[i]].x;//x
			A[i][4] = edge_point_nor[rand_index[i]].y;//y
		}


		/*SVD函数用的是英国一个大学的原程序，对其说明如下
		Given matrix a[m][n], m>=n, using svd decomposition a = p d q' to get
		p[m][n], diag d[n] and q[n][n].

		void svd(int m, int n, double **a, double **p, double *d, double **q)
		*/

		svd(M, N, ppa, ppu, pd, ppv);


		min_d_index = 0;
		for (i = 1; i < N; i++)
		{
			if (pd[i] < pd[min_d_index])//找到对角阵里最小的序号，那一个	奇异值对应的就是解的列
				min_d_index = i;
		}

		for (i = 0; i < N; i++)//这里得到的是一列(注意)
			conic_par[i] = ppv[i][min_d_index];	//the column of v that corresponds to the smallest singular value, 
												//which is the solution of the equations
		ninliers = 0;
		memset(inliers_index, 0, sizeof(int)*ep_num);

		for (i = 0; i < ep_num; i++)//这里计算用这个参数产生的误差
		{
			dis_error = conic_par[0] * edge_point_nor[i].x*edge_point_nor[i].x +
				conic_par[1] * edge_point_nor[i].x*edge_point_nor[i].y +
				conic_par[2] * edge_point_nor[i].y*edge_point_nor[i].y +
				conic_par[3] * edge_point_nor[i].x + conic_par[4] * edge_point_nor[i].y + conic_par[5];
			if (fabs(dis_error) < dis_threshold)//把小误差的点记录下来
			{
				inliers_index[ninliers] = i;//记下来小误差的点在ep_num中的位置
				ninliers++;
			}
		}

		//判断当前是否要继续这五个值产生的参数，
		//如果ninliers > max_inliers就继续滚雪球
		//否则重新产生5个点
		if (ninliers > max_inliers)
		{
			if (solve_ellipse(conic_par, ellipse_par))
			{
				denormalize_ellipse_param(ellipse_par, ellipse_par, dis_scale, nor_center);
				ratio = ellipse_par[0] / ellipse_par[1];
				if (ellipse_par[2] > 0 && ellipse_par[2] <= width - 1
					&& ellipse_par[3] > 0 && ellipse_par[3] <= height - 1
					&& ratio > 0.9 && ratio < 1.1)
				{//合适就把这些点放进 max_inliers_index 来
					memcpy(max_inliers_index, inliers_index, sizeof(int)*ep_num);
					for (i = 0; i < 5; i++)
					{
						best_ellipse_par[i] = ellipse_par[i];
					}
					max_inliers = ninliers;
					//下面是按照论文上的公式，当前记载的是最多的点集的个数，计算此概率下应该循环多少次
					sample_num = (int)(log((double)(1 - 0.99)) / log(1.0 - pow(ninliers*1.0 / ep_num, 5)));

				}
			}
		}

		ransac_count++;
		//这里的控制是说最多不能超过1500次 
		if (ransac_count > 1500)
		{
			//printf("Error! ransac_count exceed! ransac break! sample_num=%d, ransac_count=%d\n", sample_num, ransac_count);
			break;
		}
	}//对应while的结束


	 //INFO("ransc end\n");
	if (best_ellipse_par[0] > 0 && best_ellipse_par[1] > 0)
	{
		for (i = 0; i < 5; i++)
		{
			circle_param[i] = best_ellipse_par[i];
		}
	}
	else
	{
		memset(circle_param, 0, sizeof(circle_param));
		max_inliers = 0;
		free(max_inliers_index);
		max_inliers_index = NULL;
	}

	for (i = 0; i < M; i++)
	{
		free(ppu[i]);
		free(ppv[i]);
	}
	free(ppu);
	free(ppv);
	free(ppa);

	free(edge_point_nor);
	free(inliers_index);
	free(max_inliers_index);
	return_max_inliers_num = max_inliers;
	// return max_inliers_index;
}//对应函数结束

 //画十字线
void CCircle_detector::Draw_Cross(IplImage *image, int centerx, int centery, int x_cross_length, int y_cross_length, CvScalar color)
{
	CvPoint pt1, pt2, pt3, pt4;

	pt1.x = centerx - x_cross_length;
	pt1.y = centery;
	pt2.x = centerx + x_cross_length;
	pt2.y = centery;

	pt3.x = centerx;
	pt3.y = centery - y_cross_length;
	pt4.x = centerx;
	pt4.y = centery + y_cross_length;

	cvLine(image, pt1, pt2, color, 1, 8);
	cvLine(image, pt3, pt4, color, 1, 8);
}


void CCircle_detector::compute_ellipse(cv::Mat &InImage, double* in, cv::Point2f &center,cv::Point2f &corneal_center)
{
    clock_t start = clock();
    char param_str[256];
    IplImage *rgbI = &IplImage(InImage);
    IplImage * image = cvCreateImage(cvGetSize(rgbI), IPL_DEPTH_8U, 1);//单通道处理
    IplImage *threshold_image=cvCreateImage(cvGetSize(rgbI), IPL_DEPTH_8U, 1);//
    if (InImage.channels() == 3)
    {
        cvCvtColor(rgbI, image, CV_BGR2GRAY);
    }
    else
    {
        cvCopy(rgbI, image);
    }
    cvSmooth(image, image, CV_GAUSSIAN, 5, 5);


    CCorneal CorImage;
    CCircle_detector inner_circle;
    CorImage.remove_corneal_reflection(image, threshold_image, center.x, center.y,
            (int)image->height/10, CorImage.corneal_reflection.x, CorImage.corneal_reflection.y, CorImage.corneal_reflection_r);

    Draw_Cross(rgbI, CorImage.corneal_reflection.x, CorImage.corneal_reflection.y, 5, 5, CV_RGB(0,255,0));
    corneal_center.x=CorImage.corneal_reflection.x;
    corneal_center.y=CorImage.corneal_reflection.y;

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
        Draw_Cross(rgbI, pupil.x, pupil.y, 5, 5, Red);
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
        center.x=pupil.x;
        center.y=pupil.y;
    }
    rgbI = NULL;
    cvReleaseImage(&threshold_image);
    cvReleaseImage(&image);
}
