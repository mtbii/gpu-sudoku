#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/gpu/gpu.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml.hpp>

#include "DigitOCR.h"

using namespace cv;
using namespace gpu;

class Transformer{

public:
	Transformer();
	virtual ~Transformer();
	int recognize(const Mat& img);
	int recognize(const GpuMat& img);

private:
	DigitOCR* ocr;

	void preProcess(Mat& img);
	void resizeWithAspect(Mat& img, int w);
	void borderAndCenter(Mat& img, int w, int h);
};