#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/gpu/gpu.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <math.h>
#include "transformer.h"

#define PUZZLE_SIZE 9

using namespace cv;
using namespace std;
using namespace gpu;

class Filter
{
public:
	Filter();
	~Filter();
	vector<vector<int>> Filter::filterForPuzzle(Mat& frame, Mat& debug_frame, int frame_skip = 1);
	vector<vector<int>> Filter::filterForPuzzle(GpuMat& frame, GpuMat& debug_frame, int frame_skip = 1);
	static string edges_window_title;

private:
	Transformer parser;

	vector<vector<int>> extractDigits(Mat& img);
	vector<Point2f> extractPuzzle(Mat& inputImg, Mat& contour, Mat& contourImg, Mat& outputImg);
	Mat floodFillRegion(Mat& inputImg, Rect& region, int value = 255);
	void fill(Mat& img, Point2i startPt, int value, Rect& size);

	void adaptiveThreshold(GpuMat& src, GpuMat& dst,
		double maxValue, int adaptiveMethod,
		int thresholdType, int blockSize, double C);
	vector<vector<int>> extractDigits(GpuMat& img);
	vector<Point2f> extractPuzzle(GpuMat& inputImg, Mat& contour, Mat& contourImg, GpuMat& outputImg);
	GpuMat floodFillRegion(GpuMat& inputImg, Rect& region, int value = 255);
	void fill(GpuMat& img, Point2i startPt, int value, Rect& size);
};

