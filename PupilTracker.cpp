
#include "PupilTracker.h"


class HaarSurroundFeature
{
public:
	HaarSurroundFeature(int r1, int r2) :r_inner(r1), r_outer(r2)
	{
		int count_inner = r_inner*r_inner;
		int count_outer = r_outer*r_outer - r_inner*r_inner;
		val_inner = 1.0 / (r_inner*r_inner);
		val_outer = -val_inner*count_inner / count_outer;
	}
	double val_inner, val_outer;
	int r_inner, r_outer;
};


bool findPupilEllipse(const cv::Mat &m, TrackerParams &params)
{
	cv::Mat temp;
	m.copyTo(temp);
	clock_t start = clock();
	cv::Mat_<uchar> mEye;
	if (m.channels() == 1)
	{
		mEye = m;
	}
	else if (m.channels() == 3)
	{
		cv::cvtColor(m, mEye, cv::COLOR_BGR2GRAY);
	}
	else if (m.channels() == 4)
	{
		cv::cvtColor(m, mEye, cv::COLOR_BGRA2GRAY);
	}
	else
	{
		throw std::runtime_error("Unsuppported number of channels");
	}
	cv::Mat_<int32_t> mEyeIntegral;
	int padding = 2 * params.Radius_Max;

	cv::Mat mEyePad;
	cv::copyMakeBorder(mEye, mEyePad, padding, padding, padding, padding, cv::BORDER_REPLICATE);
	cv::integral(mEyePad, mEyeIntegral);

	cv::Point2f pHaarPupil;
	int haarRadius = 0;

	const int rstep = 2;
	const int ystep = 4;
	const int xstep = 4;

	double minResponse = std::numeric_limits<double>::infinity();

	for (int r = params.Radius_Min; r < params.Radius_Max; r += rstep)
	{
		// Get Haar feature
		int r_inner = r;
		int r_outer = 3 * r;
		HaarSurroundFeature f(r_inner, r_outer);

		// Use TBB for rows
		std::pair<double, cv::Point2f> minRadiusResponse = tbb::parallel_reduce(
			tbb::blocked_range<int>(0, (mEye.rows - r - r - 1) / ystep + 1, ((mEye.rows - r - r - 1) / ystep + 1) / 8),
			std::make_pair(std::numeric_limits<double>::infinity(), UNKNOWN_POSITION),
			[&](tbb::blocked_range<int> range, const std::pair<double, cv::Point2f>& minValIn) -> std::pair<double, cv::Point2f>
		{
			std::pair<double, cv::Point2f> minValOut = minValIn;
			for (int i = range.begin(), y = r + range.begin()*ystep; i < range.end(); i++, y += ystep)
			{
				//            ?        ?                    // row1_outer.|         |  p00._____________________.p01
				//            |         |     |         Haar kernel |
				//            |         |     |                     |
				// row1_inner.|         |     |   p00._______.p01   |
				//            |-padding-|     |      |       |      |
				//            |         |     |      | (x,y) |      |
				// row2_inner.|         |     |      |_______|      |
				//            |         |     |   p10'       'p11   |
				//            |         |     |                     |
				// row2_outer.|         |     |_____________________|
				//            |         |  p10'                     'p11
				//            ?        ?
				int* row1_inner = mEyeIntegral[y + padding - r_inner];
				int* row2_inner = mEyeIntegral[y + padding + r_inner + 1];
				int* row1_outer = mEyeIntegral[y + padding - r_outer];
				int* row2_outer = mEyeIntegral[y + padding + r_outer + 1];

				int* p00_inner = row1_inner + r + padding - r_inner;
				int* p01_inner = row1_inner + r + padding + r_inner + 1;
				int* p10_inner = row2_inner + r + padding - r_inner;
				int* p11_inner = row2_inner + r + padding + r_inner + 1;

				int* p00_outer = row1_outer + r + padding - r_outer;
				int* p01_outer = row1_outer + r + padding + r_outer + 1;
				int* p10_outer = row2_outer + r + padding - r_outer;
				int* p11_outer = row2_outer + r + padding + r_outer + 1;

				for (int x = r; x < mEye.cols - r; x += xstep)
				{
					int sumInner = *p00_inner + *p11_inner - *p01_inner - *p10_inner;
					int sumOuter = *p00_outer + *p11_outer - *p01_outer - *p10_outer - sumInner;

					double response = f.val_inner * sumInner + f.val_outer * sumOuter;

					if (response < minValOut.first)
					{
						minValOut.first = response;
						minValOut.second = cv::Point(x, y);
					}

					p00_inner += xstep;
					p01_inner += xstep;
					p10_inner += xstep;
					p11_inner += xstep;

					p00_outer += xstep;
					p01_outer += xstep;
					p10_outer += xstep;
					p11_outer += xstep;
				}
			}
			return minValOut;
		},
			[](const std::pair<double, cv::Point2f>& x, const std::pair<double, cv::Point2f>& y) -> std::pair<double, cv::Point2f>
		{
			if (x.first < y.first)
				return x;
			else
				return y;
		}
		);

		if (minRadiusResponse.first < minResponse)
		{
			minResponse = minRadiusResponse.first;
			// Set return values
			pHaarPupil = minRadiusResponse.second;
			haarRadius = r;
		}
	}

	// Paradoxically, a good Haar fit won't catch the entire pupil, so expand it a bit
	haarRadius = (int)(haarRadius * SQRT_2);
	params.Pupil_center.x = pHaarPupil.x*5;
	params.Pupil_center.y = pHaarPupil.y*5;

	//std::cout << double(clock() - start) / CLOCKS_PER_SEC << std::endl;

	//cv::circle(temp, pHaarPupil, haarRadius, CV_RGB(255, 0, 0), 1, 8, 0);
	//cv::imshow(" ", temp);
	//cv::waitKey(0);
	//cv::destroyAllWindows();




	return true;
}

