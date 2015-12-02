#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/gpu/gpu.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <math.h>

using namespace cv;
using namespace std;

class Filter
{
public:
	Filter();
	~Filter();
	vector<vector<int>> Filter::filterForPuzzle(Mat& frame, Mat& debug_frame, int frame_skip);

private:
	vector<vector<int>> extractDigits(Mat& img);
	void extractPuzzle(Mat& inputImg, Mat& contour, Mat& contourImg, Mat& outputImg);
};

