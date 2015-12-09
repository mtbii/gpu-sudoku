#include "Filter.h"

string Filter::edges_window_title = "Edges";

const int sigma = 100;
const int thresholdSize = 51;
const int blurKernelSize = 3;
const int threshMaxVal = 255;// 26;
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

vector<Point2f> Filter::extractPuzzle(Mat& inputImg, Mat& contour, Mat& contourImg, Mat& outputImg){
	RotatedRect obb = minAreaRect(contour);
	vector<Vec4i> lines;
	vector<Point2f> points(4, Point2f(0, 0));

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
		Point2f box[4];

		Size img_size = Size(inputImg.size().height * 2, inputImg.size().height * 2);

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

		if (abs(h1 - h2) > minLineSize * warpLimit){ return points; }
		if (abs(w1 - w2) > minLineSize* warpLimit){ return points; }

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
			Mat lambda = getPerspectiveTransform(&points[0], box);
			warpPerspective(inputImg, outputImg, lambda, outputImg.size());

			Mat cropped = outputImg(Rect(box[0], box[2])).clone();
			outputImg = Mat::zeros(outputImg.size(), outputImg.type());
			Mat cropROI = outputImg(Rect(box[0], box[2]));
			cropped.copyTo(cropROI);
		}
	}
	return points;
}

vector<Point2f> Filter::extractPuzzle(GpuMat& inputImg, Mat& contour, Mat& contourImg, GpuMat& outputImg){
	RotatedRect obb = minAreaRect(contour);
	vector<Vec4i> lines;
	vector<Point2f> points(4, Point2f(0, 0));

	GpuMat d_lines;
	GpuMat linesImg = GpuMat(contourImg);
	double minLineSize = obb.size.width * .5;

	// detect the lines
	HoughLinesBuf houghBuf;
	gpu::HoughLinesP(linesImg, d_lines, houghBuf, 1, CV_PI / 180.0, minLineSize, minLineSize * .5);

	//for (size_t i = 0; i < lines.size(); i++)
	//{
	//	Vec4i l = lines[i];
	//	// draw the lines
	//	line(inputImg, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 3, CV_AA);
	//}

	vector<Vec4i> lines_gpu;
	if (!d_lines.empty())
	{
		lines.resize(d_lines.cols);
		Mat h_lines(1, d_lines.cols, CV_32SC4, &lines[0]);
		d_lines.download(h_lines);
	}

	if (lines.size() >= 4){
		Point2f box[4];

		Size img_size = Size(inputImg.size().height * 2, inputImg.size().height * 2);

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

		if (abs(h1 - h2) > minLineSize * warpLimit){ return points; }
		if (abs(w1 - w2) > minLineSize* warpLimit){ return points; }

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
			Mat lambda = getPerspectiveTransform(&points[0], box);
			gpu::warpPerspective(inputImg, outputImg, lambda, outputImg.size());

			GpuMat cropped = outputImg(Rect(box[0], box[2])).clone();
			outputImg = GpuMat(Mat::zeros(outputImg.size(), outputImg.type()));
			GpuMat cropROI = outputImg(Rect(box[0], box[2]));
			cropped.copyTo(cropROI);
		}
	}
	return points;
}

vector<vector<int>> Filter::extractDigits(Mat& frame){
	vector<vector<int>> puzzle = vector<vector<int>>(PUZZLE_SIZE, vector<int>(PUZZLE_SIZE, 0));

	if (frame.size().area() > 0){

		//Filter image
		Mat workingImage = Mat(frame.size(), CV_8UC1);
		int w = workingImage.size().width;
		int h = workingImage.size().height;

		double offset = 0.37;
		double size = 1.0 - 2 * offset;

		int blockSize = w / PUZZLE_SIZE;

		if (blockSize % 2 == 0) blockSize++;

		cvtColor(frame, workingImage, CV_RGB2GRAY);
		cv::adaptiveThreshold(workingImage, workingImage, 127, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, blockSize, C);

		//Find digits
		Mat squares[PUZZLE_SIZE][PUZZLE_SIZE];

		for (int i = 0; i < PUZZLE_SIZE; i++){
			for (int j = 0; j < PUZZLE_SIZE; j++){
				squares[i][j] = workingImage(Rect(i*w / PUZZLE_SIZE, j*h / PUZZLE_SIZE, w / PUZZLE_SIZE, h / PUZZLE_SIZE));

				Mat square = squares[i][j];
				int w_s = square.size().width;
				int h_s = square.size().height;

				Rect fillRegion = Rect(i*w_s + offset*w / PUZZLE_SIZE, j*h_s + offset*h / PUZZLE_SIZE, size*w / PUZZLE_SIZE, size*h / PUZZLE_SIZE);
				squares[i][j] = floodFillRegion(workingImage, fillRegion);

				vector<Point2i> points(4, Point2i(0, 0));
				points[0] = Point2i(fillRegion.x, fillRegion.y);
				points[1] = Point2i(fillRegion.x + fillRegion.width, fillRegion.y);
				points[2] = Point2i(fillRegion.x + fillRegion.width, fillRegion.y + fillRegion.height);
				points[3] = Point2i(fillRegion.x, fillRegion.y + fillRegion.height);

				if (fillRegion.size().width != (int)(size*w / PUZZLE_SIZE) && fillRegion.size().height != (int)(size*h / PUZZLE_SIZE)){
					puzzle[i][j] = parser.recognize(squares[i][j]);
				}
			}
		}
	}

	return puzzle;
}

vector<vector<int>> Filter::extractDigits(GpuMat& frame){
	vector<vector<int>> puzzle = vector<vector<int>>(PUZZLE_SIZE, vector<int>(PUZZLE_SIZE, 0));

	if (frame.size().area() > 0){

		//Filter image
		GpuMat workingImage = GpuMat(Mat(frame.size(), CV_8UC1));
		int w = workingImage.size().width;
		int h = workingImage.size().height;

		double offset = 0.37;
		double size = 1.0 - 2 * offset;

		int blockSize = w / PUZZLE_SIZE;

		if (blockSize % 2 == 0) blockSize++;

		cvtColor(frame, workingImage, CV_RGB2GRAY);
		adaptiveThreshold(workingImage, workingImage, 127, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, blockSize, C);

		//Find digits
		GpuMat squares[PUZZLE_SIZE][PUZZLE_SIZE];

		for (int i = 0; i < PUZZLE_SIZE; i++){
			for (int j = 0; j < PUZZLE_SIZE; j++){
				squares[i][j] = workingImage(Rect(i*w / PUZZLE_SIZE, j*h / PUZZLE_SIZE, w / PUZZLE_SIZE, h / PUZZLE_SIZE));

				GpuMat square = squares[i][j];
				int w_s = square.size().width;
				int h_s = square.size().height;

				Rect fillRegion = Rect(i*w_s + offset*w / PUZZLE_SIZE, j*h_s + offset*h / PUZZLE_SIZE, size*w / PUZZLE_SIZE, size*h / PUZZLE_SIZE);
				squares[i][j] = floodFillRegion(workingImage, fillRegion);

				vector<Point2i> points(4, Point2i(0, 0));
				points[0] = Point2i(fillRegion.x, fillRegion.y);
				points[1] = Point2i(fillRegion.x + fillRegion.width, fillRegion.y);
				points[2] = Point2i(fillRegion.x + fillRegion.width, fillRegion.y + fillRegion.height);
				points[3] = Point2i(fillRegion.x, fillRegion.y + fillRegion.height);

				if (fillRegion.size().width != (int)(size*w / PUZZLE_SIZE) && fillRegion.size().height != (int)(size*h / PUZZLE_SIZE)){
					puzzle[i][j] = parser.recognize(squares[i][j]);
					//stringstream ss;
					//ss << puzzle[i][j];
					//putText(frame, ss.str(), Point(fillRegion.x, fillRegion.y), FONT_HERSHEY_PLAIN, 2, Scalar(0, 0, 255));
				}
			}
		}
	}

	return puzzle;
}

Mat Filter::floodFillRegion(Mat& inputImg, Rect& region, int value){
	Rect size = region;
	bool found = false;
	Mat ffImg = inputImg;

	for (int i = region.x; i < region.x + region.width; i++){
		for (int j = region.y; j < region.y + region.height; j++){
			uchar val = ffImg.at<uchar>(j, i);
			if (val > 0){
				fill(ffImg, Point(i, j), value, size);
				found = true;
				break;
			}
		}

		if (found) break;
	}

	Mat floodImage = inputImg(size);
	region = size;
	return floodImage;
}

GpuMat Filter::floodFillRegion(GpuMat& inputImg, Rect& region, int value){
	Rect size = region;
	bool found = false;
	Mat ffImg(inputImg);

	for (int i = region.x; i < region.x + region.width; i++){
		for (int j = region.y; j < region.y + region.height; j++){
			uchar val = ffImg.at<uchar>(j, i);
			if (val > 0){
				fill(ffImg, Point(i, j), value, size);
				found = true;
				break;
			}
		}

		if (found) break;
	}

	GpuMat floodImage(inputImg(size));
	region = size;
	return floodImage;
}

void Filter::fill(Mat& img, Point2i startPt, int value, Rect& size){
	int minX = img.size().width;
	int minY = img.size().height;
	int maxX = 0;
	int maxY = 0;

	Mat visited = Mat::zeros(img.size(), CV_8UC1);

	uchar val = img.at<uchar>(startPt);

	queue<Point2i> pixelQueue;
	pixelQueue.push(startPt);

	while (!pixelQueue.empty()) {
		Point2i pt = pixelQueue.front();
		pixelQueue.pop();

		if (pt.x >= 0 && pt.x < img.size().width && pt.y >= 0 && pt.y < img.size().height){

			uchar visPtr = visited.at<uchar>(pt);
			if (visPtr == 0){

				visited.at<uchar>(pt) = 1;
				uchar px = img.at<uchar>(pt);

				if (px == val){
					img.at<uchar>(pt) = value;
					if (pt.x < minX){
						minX = pt.x;
					}

					if (pt.x > maxX){
						maxX = pt.x;
					}

					if (pt.y < minY){
						minY = pt.y;
					}

					if (pt.y > maxY){
						maxY = pt.y;
					}

					pixelQueue.push(Point2i(pt.x - 1, pt.y));
					pixelQueue.push(Point2i(pt.x + 1, pt.y));
					pixelQueue.push(Point2i(pt.x, pt.y - 1));
					pixelQueue.push(Point2i(pt.x, pt.y + 1));
				}
			}
		}
	}

	size = Rect(minX, minY, maxX - minX + 1, maxY - minY + 1);
}

void Filter::fill(GpuMat& frame, Point2i startPt, int value, Rect& size){
	Mat img(frame);
	int minX = img.size().width;
	int minY = img.size().height;
	int maxX = 0;
	int maxY = 0;

	Mat visited = Mat::zeros(img.size(), CV_8UC1);
	uchar val = img.at<uchar>(startPt);

	queue<Point2i> pixelQueue;
	pixelQueue.push(startPt);

	while (!pixelQueue.empty()) {
		Point2i pt = pixelQueue.front();
		pixelQueue.pop();

		if (pt.x >= 0 && pt.x < img.size().width && pt.y >= 0 && pt.y < img.size().height){

			uchar visPtr = visited.at<uchar>(pt);
			if (visPtr == 0){

				visited.at<uchar>(pt) = 1;
				uchar px = img.at<uchar>(pt);

				if (px == val){
					img.at<uchar>(pt) = value;
					if (pt.x < minX){
						minX = pt.x;
					}

					if (pt.x > maxX){
						maxX = pt.x;
					}

					if (pt.y < minY){
						minY = pt.y;
					}

					if (pt.y > maxY){
						maxY = pt.y;
					}

					pixelQueue.push(Point2i(pt.x - 1, pt.y));
					pixelQueue.push(Point2i(pt.x + 1, pt.y));
					pixelQueue.push(Point2i(pt.x, pt.y - 1));
					pixelQueue.push(Point2i(pt.x, pt.y + 1));
				}
			}
		}
	}

	size = Rect(minX, minY, maxX - minX + 1, maxY - minY + 1);
}

vector<vector<int>> Filter::filterForPuzzle(Mat& input_frame, Mat& debug_frame, int frame_skip){
	static Mat drawing;
	static Mat frame_puzzle;
	static int frame_index = 0;
	static vector<Point2f> puzzlePoints = vector<Point2f>(4, Point2f(0, 0));

	Mat frame = input_frame.clone();
	Mat frame_filtered = frame.clone();
	Size image_size = frame.size();
	vector<vector<int>> puzzle;

	if (frame_index % frame_skip == 0) {
		cvtColor(frame, frame, CV_BGR2GRAY);
		GaussianBlur(frame, frame, Size(blurKernelSize, blurKernelSize), (double)sigma / 100.0, (double)sigma / 100.0);
		//adaptiveBilateralFilter(frame_gray, frame_blur, Size(blurKernelSize, blurKernelSize), sigma / 100.0);
		cv::adaptiveThreshold(frame, frame, threshMaxVal, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, thresholdSize, C);

		morphologyEx(frame, frame, MORPH_OPEN, cross, Point(-1, -1), 1);
		morphologyEx(frame, frame, MORPH_DILATE, cross, Point(-1, -1), 3);
		morphologyEx(frame, frame_filtered, MORPH_ERODE, cross, Point(-1, -1), 1);
		//morphologyEx(frame_threshold, frame_threshold, MORPH_DILATE, cross, Point(-1, -1), 1);

		Canny(frame_filtered, frame_filtered, lowThreshold, highThreshold);
		debug_frame = frame_filtered;
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

			Mat lastDetectedContour = Mat(contours[biggestContourId]).clone();
			puzzlePoints = extractPuzzle(input_frame, lastDetectedContour, drawing, frame_puzzle);
			puzzle = extractDigits(frame_puzzle);

			debug_frame = drawing;
			double h = debug_frame.size().height;
			double w = debug_frame.size().width;
			double divisions = PUZZLE_SIZE;
			double step = w / divisions;

			for (int i = 0; i < divisions - 1; i++){
				line(debug_frame, Point2f((i + 1)*step, 0), Point2f((i + 1)*step, h), Scalar(0, 0, 255), (i + 1) % 10 != 0 ? 1 : 3);
			}

			for (int i = 0; i < divisions - 1; i++){
				line(debug_frame, Point2f(0, (i + 1)*step), Point2f(w, (i + 1)*step), Scalar(0, 0, 255), (i + 1) % 10 != 0 ? 1 : 3);
			}
		}


		for (int j = 0; j < 4; j++){
			line(input_frame, puzzlePoints[j], puzzlePoints[(j + 1) % 4], Scalar(0, 255, 0));
		}
	}

	frame_index++;
	debug_frame = frame_puzzle;
	return puzzle;
}

vector<vector<int>> Filter::filterForPuzzle(GpuMat& input_frame, GpuMat& debug_frame, int frame_skip){
	static Mat drawing;
	GpuMat d_frame_puzzle;
	static int frame_index = 0;
	static vector<Point2f> puzzlePoints = vector<Point2f>(4, Point2f(0, 0));

	GpuMat d_frame = input_frame.clone();
	GpuMat d_frame_filtered = d_frame.clone();
	Size image_size = d_frame.size();
	vector<vector<int>> puzzle;

	if (frame_index % frame_skip == 0) {
		cvtColor(d_frame.clone(), d_frame, CV_BGR2GRAY);
		GaussianBlur(d_frame.clone(), d_frame, Size(blurKernelSize, blurKernelSize), (double)sigma / 100.0, (double)sigma / 100.0);
		adaptiveThreshold(d_frame.clone(), d_frame, threshMaxVal, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, thresholdSize, C);

		morphologyEx(d_frame.clone(), d_frame, MORPH_OPEN, cross, Point(-1, -1), 1);
		morphologyEx(d_frame.clone(), d_frame, MORPH_DILATE, cross, Point(-1, -1), 3);
		morphologyEx(d_frame, d_frame_filtered, MORPH_ERODE, cross, Point(-1, -1), 1);

		Canny(d_frame_filtered.clone(), d_frame_filtered, lowThreshold, highThreshold);
		drawing = Mat::zeros(image_size, CV_8UC1);

		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;

		Mat frame = Mat(d_frame);

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
			Scalar color = Scalar(255);
			drawContours(drawing, contours, biggestContourId, color, 2, 8, hierarchy, 0, Point());

			Mat lastDetectedContour = Mat(contours[biggestContourId]).clone();
			puzzlePoints = extractPuzzle(input_frame, lastDetectedContour, drawing, d_frame_puzzle);
			puzzle = extractDigits(d_frame_puzzle);
		}
	}

	frame_index++;
	debug_frame = d_frame_puzzle;
	return puzzle;
}

void Filter::adaptiveThreshold(GpuMat& src, GpuMat& dst, double maxValue, int adaptiveMethod, int thresholdType, int blockSize, double C){
	int horz = (blockSize - 1) / 2;
	int vert = (blockSize - 1) / 2;

	GpuMat img, expanded;
	src.assignTo(img, CV_8UC1);
	copyMakeBorder(img, expanded, horz, horz, vert, vert, BORDER_CONSTANT, 0);

	/*Mat kernel = Mat::ones(Size(blockSize, blockSize), CV_32FC1);
	kernel = (1.0 / (blockSize*blockSize)) * kernel;
	convolve(expanded, GpuMat(kernel), thresholdMat);*/
	GpuMat thresholdMat = GpuMat(img.size(), img.type());
	boxFilter(expanded, thresholdMat, -1, Size(blockSize, blockSize));

	compare(img, thresholdMat(Rect(Point(horz, vert), src.size())), dst, CMP_LT);
	multiply(dst.clone(), maxValue, dst);
	subtract(dst.clone(), C, dst);
}