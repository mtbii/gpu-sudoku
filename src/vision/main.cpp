#include "main.h"

int main(int argc, char** argv)
{
	VideoCapture capture("D:/Projects/sudoku_capture.mp4");
	Size window_size = Size(WINDOW_WIDTH, WINDOW_HEIGHT);
	Size image_size = Size(window_size.width / 2, window_size.height / 2);

	const string window_name = "Sudoku Capture";
	namedWindow(window_name, WINDOW_AUTOSIZE);

	const string debug_window_name = "Filtered Capture";
	namedWindow(debug_window_name, WINDOW_AUTOSIZE);

	if (!capture.isOpened()){
		cout << "Cannot open video stream" << endl;
		return -1;
	}

	Mat frame, frame_gray, frame_blur, frame_threshold, frame_hough, frame_puzzle, lastDetectedContour = Mat();
	double frame_rate = capture.get(CV_CAP_PROP_FPS) == 0 ? 30 : capture.get(CV_CAP_PROP_FPS);

	double contour_rate = 1;
	long frame_skip = (long)(frame_rate / contour_rate);
	long frame_index = 0;
	double delay = 1000.0 / frame_rate;

	if (!capture.read(frame)){
		cout << "Error opening file/device." << endl;
		exit(EXIT_FAILURE);
	}

	Mat frame_output = Mat(window_size.height, window_size.width, frame.type());
	Mat drawing = Mat::zeros(image_size, CV_8UC1);
	Filter filter;
	//SudokuPuzzle sudoku_puzzle;
	//SudokuSolver solver;

	while (true){

		if (!capture.read(frame)){
			//Restart video
			capture.set(CV_CAP_PROP_POS_FRAMES, 0);
			capture.read(frame);
		}

		resize(frame, frame, image_size);
		vector<vector<int>> puzzle = filter.filterForPuzzle(frame, drawing, frame_skip);
		//sudoku_puzzle = SudokuPuzzle(puzzle);
		//sudoku_puzzle = solver.solve(puzzle);
		//showPuzzle

		//frame_index++;

		imshow(window_name, frame);
		imshow(debug_window_name, drawing);

		if (waitKey(delay) >= 0) break;
	}

	return 0;
}
