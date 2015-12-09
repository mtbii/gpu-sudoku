#include "DigitOCR.h"

int DigitOCR::samples = 6;
int DigitOCR::classes = 9;
int DigitOCR::sizeX = 20;
int DigitOCR::sizeY = 30;
string DigitOCR::filepath = "train/";

DigitOCR::DigitOCR(){
	mlEngine = new KNearest();

	Mat images = Mat::zeros(classes*samples, sizeX*sizeY, CV_32FC1);
	Mat labels = Mat::zeros(classes*samples, 1, CV_32FC1);

	for (int i = 0; i < classes; i++){
		for (int j = 0; j < samples; j++){
			stringstream ss;

			//Starts at number 1
			ss << filepath << (i + 1) << "_" << j;// << ".png";
			string file = ss.str() + ".png";
			Mat img = imread(file);
			Mat outImg;

			preProcess(img, outImg, sizeX, sizeY, true);

			//imwrite("proc/" + ss.str() + ".bmp", outImg.reshape(1, sizeY));

			for (int k = 0; k < images.size().width; k++){
				images.at<float>(i*samples + j, k) = outImg.at<float>(0, k);
			}

			labels.at<float>(i*samples + j, 0) = i + 1;
		}
	}

	train(images, labels);
}

DigitOCR::~DigitOCR(){
	if (mlEngine != nullptr){
		delete mlEngine;
		mlEngine = nullptr;
	}
}

int DigitOCR::predict(const Mat& img){
	//Put checks
	Mat sample;
	preProcess(img, sample, sizeX, sizeY, false);
	//imwrite("proc/out.bmp", sample.reshape(1, sizeY));

	Mat results;
	mlEngine->find_nearest(sample, 1, results, Mat(), Mat());
	int val = (int)results.at<float>(0, 0);
	return val;
}

void DigitOCR::train(const Mat& images, const Mat& labels){
	//Check size and stuff
	mlEngine->train(images, labels);
}

void DigitOCR::preProcess(const Mat& inImage, Mat& outImage, int sizex, int sizey, bool invert)
{
	Mat grayImage, blurredImage, thresholdImage, contourImage, regionOfInterest;

	vector<vector<Point> > contours;

	try{
		cvtColor(inImage, grayImage, COLOR_BGR2GRAY);
	}
	catch (exception& ex){
		grayImage = inImage.clone();
	}

	GaussianBlur(grayImage, blurredImage, Size(5, 5), 2, 2);
	//imwrite("proc/mid.bmp", blurredImage);
	//adaptiveThreshold(blurredImage, thresholdImage, 255, CV_ADAPTIVE_THRESH_MEAN_C, 1, 11, 2);
	threshold(blurredImage, thresholdImage, 128, 255, CV_THRESH_OTSU);

	if (invert)
		thresholdImage = Scalar::all(255) - thresholdImage;

	//imwrite("proc/mid.bmp", thresholdImage);
	thresholdImage.copyTo(contourImage);

	findContours(contourImage, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

	int idx = 0;
	size_t area = 0;
	for (size_t i = 0; i < contours.size(); i++)
	{
		if (area < contourArea(contours[i]))
		{
			idx = i;
			area = contourArea(contours[i]);
		}
	}

	Rect rec = boundingRect(contours[idx]);

	regionOfInterest = thresholdImage(rec);
	resize(regionOfInterest, outImage, Size(sizex, sizey));
	//imwrite("proc/mid.bmp", outImage);

	outImage.assignTo(outImage, CV_32FC1);

	outImage = outImage.reshape(1, 1);
}

void DigitOCR::save(const string& filePath){
	mlEngine->save(filePath.c_str(), "digitOCR");
}

void DigitOCR::load(const string& filePath){
	mlEngine->load(filePath.c_str(), "digitOCR");
}