#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <opencv2/core.hpp>
#include <opencv2/ml.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/gpu/gpu.hpp>

using namespace std;
using namespace cv;
using namespace gpu;

class DigitOCR
{
public:
	static int samples;
	static int classes;
	static int sizeX;
	static int sizeY;
	static string filepath;

	DigitOCR();
	virtual ~DigitOCR();
	int predict(const Mat& sample);
	void save(const string& filePath);
	void load(const string& filePath);

private:
	void train(const Mat& images, const Mat& labels);
	void preProcess(const Mat& inImage, Mat& outImage, int sizex, int sizey, bool invert);
	KNearest* mlEngine;
};

