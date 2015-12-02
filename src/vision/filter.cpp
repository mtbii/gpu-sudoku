#include "Filter.h"

const int sigma = 100;
const int thresholdSize = 51;
const int blurKernelSize = 3;
const int threshMaxVal = 26;
const int lowThreshold = 0;
const int highThreshold = 128;
const int C = 2;

const Mat cross = getStructuringElement(CV_SHAPE_CROSS, Size(3, 3));
const Mat rect = getStructuringElement(CV_SHAPE_RECT, Size(3, 3));

RNG rng(12345);
Filter::Filter()
{
}

Filter::~Filter()
{
}

//void showImages(Size& image_size, Mat& img0, Mat& img1, Mat& img2, Mat& img3, Mat& output){
//	resize(img0, img0, image_size);
//	resize(img1, img1, image_size);
//	resize(img2, img2, image_size);
//	resize(img3, img3, image_size);
//
//	Rect roi = Rect(0, 0, image_size.width, image_size.height);
//	Mat targetROI = output(roi);
//	img0.copyTo(targetROI);
//
//	roi.x = image_size.width;
//	roi.y = 0;
//	targetROI = output(roi);
//
//	try{
//		cvtColor(img1, img1, CV_GRAY2BGR);
//	}
//	catch (exception& e){}
//	img1.copyTo(targetROI);
//
//	roi.x = 0;
//	roi.y = image_size.height;
//	targetROI = output(roi);
//
//	try{
//		cvtColor(img2, img2, CV_GRAY2BGR);
//	}
//	catch (exception& e){}
//	img2.copyTo(targetROI);
//
//	roi.x = image_size.width;
//	roi.y = image_size.height;
//	targetROI = output(roi);
//
//	try{
//		cvtColor(img3, img3, CV_GRAY2BGR);
//	}
//	catch (exception& e){}
//	img3.copyTo(targetROI);
//
//	imshow("Sudoku Capture", output);
//}



void Filter::extractPuzzle(Mat& inputImg, Mat& contour, Mat& contourImg, Mat& outputImg){
	RotatedRect obb = minAreaRect(contour);
	vector<Vec4i> lines;

	Mat linesImg = contourImg.clone();
	double minLineSize = obb.size.width * .5;

	// detect the lines
	HoughLinesP(linesImg, lines, 1, CV_PI / 180, minLineSize, minLineSize, minLineSize * .5);

	//for (size_t i = 0; i < lines.size(); i++)
	//{
	//	Vec4i l = lines[i];
	//	// draw the lines
	//	line(inputImg, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 3, CV_AA);
	//}

	if (lines.size() >= 4){
		Point2f points[4];
		Point2f box[4];

		Size img_size = Size(inputImg.size().height*2, inputImg.size().height*2);

		//calculate four corners of quad
		Point2f tl, tr, bl, br;
		Point2f center = obb.center;

		for (int i = 0; i < lines.size(); i++){
			Point2f p1 = Point2f(lines[i][0], lines[i][1]);
			Point2f p2 = Point2f(lines[i][2], lines[i][3]);

			Point2f cp1 = p1 - center;
			Point2f cp2 = p2 - center;

			if (cp1.x <= 0 && cp1.y < 0){
				if (cp1.dot(cp1) > tl.dot(tl)){
					tl = cp1;
				}
			}

			if (cp1.x <= 0 && cp1.y >= 0){
				if (cp1.dot(cp1) > bl.dot(bl)){
					bl = cp1;
				}
			}

			if (cp1.x > 0 && cp1.y < 0){
				if (cp1.dot(cp1) > tr.dot(tr)){
					tr = cp1;
				}
			}

			if (cp1.x > 0 && cp1.y >= 0){
				if (cp1.dot(cp1) > br.dot(br)){
					br = cp1;
				}
			}

			if (cp2.x <= 0 && cp2.y < 0){
				if (cp2.dot(cp2) > tl.dot(tl)){
					tl = cp2;
				}
			}

			if (cp2.x <= 0 && cp2.y >= 0){
				if (cp2.dot(cp2) > bl.dot(bl)){
					bl = cp2;
				}
			}

			if (cp2.x > 0 && cp2.y < 0){
				if (cp2.dot(cp2) > tr.dot(tr)){
					tr = cp2;
				}
			}

			if (cp2.x > 0 && cp2.y >= 0){
				if (cp2.dot(cp2) > br.dot(br)){
					br = cp2;
				}
			}
		}

		tl += center;
		tr += center;
		br += center;
		bl += center;

		double offset = 0;// (img_size.width - img_size.height) / 2.0;

		points[0] = tl;
		points[1] = tr;
		points[2] = br;
		points[3] = bl;

		//Quality check
		double h1 = bl.y - tl.y;
		double h2 = br.y - tr.y;
		double w1 = tr.x - tl.x;
		double w2 = br.x - bl.x;
		double warpLimit = .4; //Allowable deviation in difference of lengths of opposite sides of quad

		if (abs(h1 - h2) > minLineSize * warpLimit){ return; }
		if (abs(w1 - w2) > minLineSize* warpLimit){ return; }

		//for (int i = 0; i < 4; i++){
		//	if (points[i].x == 0 && points[i].y == 0){
		//		return; //One point was not assigned, throw away result
		//	}

		//	for (int j = i + 1; j < 4; j++){
		//		if (points[i] == points[j]){
		//			return; //Points have collapsed, not a square, points must be distinct
		//		}
		//	}
		//}

		box[0] = Point2f(offset, 0);
		box[1] = Point2f(img_size.height + offset, 0);
		box[2] = Point2f(img_size.height + offset, img_size.height);
		box[3] = Point2f(offset, img_size.height);

		if (addressof(inputImg) != addressof(outputImg)){
			//outputImg = Mat(img_size, inputImg.type());
			resize(inputImg.clone(), outputImg, img_size);
			Mat lambda = getPerspectiveTransform(points, box);
			warpPerspective(inputImg, outputImg, lambda, outputImg.size());

			Mat cropped = outputImg(Rect(box[0], box[2])).clone();
			outputImg = Mat::zeros(outputImg.size(), outputImg.type());
			Mat cropROI = outputImg(Rect(box[0], box[2]));
			cropped.copyTo(cropROI);
		}

		for (int j = 0; j < 4; j++)
			line(inputImg, points[j], points[(j + 1) % 4], Scalar(0, 255, 0));
	}
}

vector<vector<int>> Filter::extractDigits(Mat& frame){
	try{
		vector<Vec4i> hierarchy;
		vector<vector<Point>> contours;

		try{
			cvtColor(frame, frame, CV_BGR2GRAY);
			GaussianBlur(frame, frame, Size(blurKernelSize, blurKernelSize), (double)sigma / 100.0, (double)sigma / 100.0);
			//adaptiveBilateralFilter(frame_gray, frame_blur, Size(blurKernelSize, blurKernelSize), sigma / 100.0);
			adaptiveThreshold(frame, frame, threshMaxVal, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, thresholdSize, C);

			morphologyEx(frame, frame, MORPH_CLOSE, rect, Point(-1, -1), 2);
			Canny(frame, frame, lowThreshold, highThreshold);
		}
		catch (exception& e){}

		/// Find contours
		findContours(frame.clone(), contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

		/// Get the moments
		vector<Moments> mu(contours.size());
		for (int i = 0; i < contours.size(); i++)
		{
			mu[i] = moments(contours[i], false);
		}

		///  Get the mass centers:
		vector<Point2f> mc(contours.size());
		for (int i = 0; i < contours.size(); i++)
		{
			mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
		}

		/// Draw contours
		frame = Mat::zeros(frame.size(), CV_8UC3);

		double frame_area = frame.size().height * frame.size().height;
		for (int i = 0; i < contours.size(); i++)
		{
			double area = contourArea(contours[i]);
			//if (area > frame.size().height / 10.0 && area < .005*frame_area){
			Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
			drawContours(frame, contours, i, color, 1, 8, hierarchy, 0, Point());
			circle(frame, mc[i], 4, color, -1, 8, 0);
			//}
		}


		//double frame_area = frame.size().height * frame.size().height;
		//for (int i = 0; i < contours.size(); i++){
		//	double area = contourArea(contours[i]);
		//	//if (area > 50 && area < .005*frame_area){
		//		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		//		//drawContours(frame, contours, i, color, 1, 8, hierarchy, 0, Point());

		//	//}
		//}
	}
	catch (exception& e){}

	return vector<vector<int>>(9, vector<int>(9, 0));
}

vector<vector<int>> Filter::filterForPuzzle(Mat& input_frame, Mat& debug_frame, int frame_skip = 1){
	static Mat drawing;
	static Mat frame_puzzle;
	static int frame_index = 0;


	Mat frame = input_frame.clone();
	Mat frame_filtered = frame.clone();
	Size image_size = frame.size();
	vector<vector<int>> puzzle;

	if (frame_index % frame_skip == 0) {
		cvtColor(frame, frame, CV_BGR2GRAY);
		GaussianBlur(frame, frame, Size(blurKernelSize, blurKernelSize), (double)sigma / 100.0, (double)sigma / 100.0);
		//adaptiveBilateralFilter(frame_gray, frame_blur, Size(blurKernelSize, blurKernelSize), sigma / 100.0);
		adaptiveThreshold(frame, frame, threshMaxVal, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, thresholdSize, C);

		morphologyEx(frame, frame, MORPH_OPEN, cross, Point(-1, -1), 1);
		morphologyEx(frame, frame, MORPH_DILATE, cross, Point(-1, -1), 3);
		morphologyEx(frame, frame_filtered, MORPH_ERODE, cross, Point(-1, -1), 1);
		//morphologyEx(frame_threshold, frame_threshold, MORPH_DILATE, cross, Point(-1, -1), 1);

		Canny(frame_filtered, frame_filtered, lowThreshold, highThreshold);
		//HoughCircles(frame_hough, circles, CV_HOUGH_GRADIENT, 4, 10, highThreshold, 100, 1, 20);
		drawing = Mat::zeros(image_size, CV_8UC1);
		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;

		/// Find contours
		findContours(frame, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
		int biggestContourId = 0;
		int mostChildren = 0;

		//Find the contour with the most direct child contours
		vector<int> childCounts = vector<int>(hierarchy.size());

		for (int i = 0; i < hierarchy.size(); i++){
			if (hierarchy[i][3] >= 0){
				childCounts[hierarchy[i][3]]++;
			}
		}

		for (int i = 0; i < contours.size(); i++){
			if (childCounts[i] > mostChildren){
				biggestContourId = i;
				mostChildren = childCounts[i];
			}
		}

		if (contours.size() > 0) {
			//drawing = Mat::zeros(image_size, CV_8UC3);
			//for (int i = 0; i < contours.size(); i++){
			Scalar color = Scalar(255); // : Scalar(0, (100 * childCounts[i]) + 10, 0); // Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
			drawContours(drawing, contours, biggestContourId, color, 2, 8, hierarchy, 0, Point());
			//}

			static Mat lastDetectedContour = Mat(contours[biggestContourId]).clone();
			extractPuzzle(input_frame, lastDetectedContour, drawing, frame_puzzle);
			puzzle = extractDigits(frame_puzzle);

			debug_frame = frame_puzzle;
			double h = debug_frame.size().height;
			double w = debug_frame.size().width;
			double step = w / 9;

			for (int i = 0; i < 8; i++){
				line(debug_frame, Point2f((i + 1)*step, 0), Point2f((i + 1)*step, h), Scalar(0, 0, 255));
			}

			for (int i = 0; i < 8; i++){
				line(debug_frame, Point2f(0, (i + 1)*step), Point2f(w, (i + 1)*step), Scalar(0, 0, 255));
			}
		}
	}
	frame_index++;
	debug_frame = frame_puzzle;
	return puzzle;
}