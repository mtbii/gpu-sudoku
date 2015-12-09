#include "main.h"

int main(int argc, char** argv)
{
	setDevice(0);
	VideoCapture capture("D:/Projects/sudoku_capture.mp4");
	Size window_size = Size(WINDOW_WIDTH, WINDOW_HEIGHT);
	Size image_size = Size(window_size.width / 2, window_size.height / 2);

	const string window_name = "Sudoku Capture";
	namedWindow(window_name, WINDOW_AUTOSIZE);

	//namedWindow(Filter::edges_window_title, WINDOW_AUTOSIZE);

	const string debug_window_name = "Filtered Capture";
	namedWindow(debug_window_name, WINDOW_AUTOSIZE);

	if (!capture.isOpened()){
		cout << "Cannot open video stream" << endl;
		return -1;
	}

	Mat frame, drawing;
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
	Mat drawing_output = Mat::zeros(image_size, CV_8UC1);

	GpuMat d_frame(frame_output);
	GpuMat d_drawing(drawing_output);

	Filter filter;
	SudokuSolver solver;

	while (true){

		if (!capture.read(frame)){
			//Restart video
			capture.set(CV_CAP_PROP_POS_FRAMES, 0);
			capture.read(frame);
		}

		resize(frame, frame, image_size);

		d_frame = GpuMat(frame);
		d_frame.upload(frame);

		//vector<vector<int>> puzzle1 = filter.filterForPuzzle(frame, drawing, frame_skip);
		vector<vector<int>> puzzle1 = filter.filterForPuzzle(d_frame, d_drawing, frame_skip);

		if (puzzle1.size() > 0) {
			puzzle1 = solver.solve(puzzle1);
			//showPuzzle
			if (!d_drawing.empty()){
				d_drawing.download(drawing_output);
				//frame_index++;

				double height = drawing_output.size().height / PUZZLE_SIZE;
				double width = drawing_output.size().width / PUZZLE_SIZE;

				for (int i = 0; i < PUZZLE_SIZE; i++){
					double y = i*height + height * .5;

					for (int j = 0; j < PUZZLE_SIZE; j++){
						double x = j*width + width * .5;

						stringstream ss;
						ss << puzzle1[i][j];
						putText(drawing_output, ss.str(), Point(x, y), FONT_HERSHEY_PLAIN, 2, Scalar(0, 0, 255));
					}
				}


				imshow(debug_window_name, drawing_output);
			}
		}
		if (!d_frame.empty()){
			d_frame.download(frame_output);
			imshow(window_name, frame_output);
		}

		if (waitKey(delay) >= 0) break;
	}

	return 0;
}
