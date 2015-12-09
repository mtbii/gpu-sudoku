#include "transformer.h"

void PreProcessImage(const Mat& inImage, Mat& outImage, int sizex, int sizey)
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
	adaptiveThreshold(blurredImage, thresholdImage, 255, 1, 1, 11, 2);
	thresholdImage.copyTo(contourImage);

	findContours(contourImage, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

	int idx = 0;
	size_t area = 0;
	for (size_t i = 0; i < contours.size(); i++)
	{
		if (area < contours[i].size())
		{
			idx = i;
			area = contours[i].size();
		}
	}

	Rect rec = boundingRect(contours[idx]);

	regionOfInterest = thresholdImage(rec);

	resize(regionOfInterest, outImage, Size(sizex, sizey));

}


Transformer::Transformer(){
	//Mat layerSizes = Mat::zeros(3, 1, CV_32SC1);
	//layerSizes.at<int>(0, 0) = DigitOCR::sizeX * DigitOCR::sizeY;
	//layerSizes.at<int>(1, 0) = 500;
	//layerSizes.at<int>(2, 0) = DigitOCR::classes;

	ocr = new DigitOCR();

	//ocr->load("train/ocr.yml");
}

Transformer::~Transformer(){
	delete ocr;
}

int Transformer::recognize(const Mat& img) {
	return ocr->predict(img);
}

int Transformer::recognize(const GpuMat& img){
	Mat img_host;
	img.download(img_host);
	return ocr->predict(img_host);
}

//void Transformer::resizeWithAspect(Mat& img, int w) {
//	Size s = img.size();
//	double aspectRatio = (double)s.width / (double)s.height;
//	resize(img, img, Size(w*aspectRatio, w));
//}
//
//void Transformer::borderAndCenter(Mat& img, int w, int h) {
//	Mat finalImg = Mat::zeros(Size(w, h), CV_32FC1);
//	int x = abs(img.size().width - w) / 2;
//	int y = abs(img.size().height - h) / 2;
//
//	Mat roi = finalImg(Rect(Point(x, y), img.size()));
//	img.assignTo(img, CV_32FC1);
//	img.copyTo(roi);
//
//	//imwrite("data/digit.bmp", img);
//	//imwrite("data/digit-fitted.bmp", finalImg);
//	img = finalImg;
//}
//
//void Transformer::preProcess(Mat& img) {
//	//resizeWithAspect(img, 20);
//
//	resize(img, img, Size(sizex, sizey));
//	GaussianBlur(img, img, Size(5, 5), 2, 2);
//	adaptiveThreshold(img, img, 255, 1, 1, 11, 2);
//	//threshold(img, img, 128, 255, CV_THRESH_BINARY);
//	borderAndCenter(img, sizex, sizey);
//	img = img.reshape(1, 1);
//}