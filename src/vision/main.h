#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/gpu/gpu.hpp>

#include "Filter.h"
#include "../sudoku/SudokuSolver.h"

#define WINDOW_HEIGHT 720
#define WINDOW_WIDTH (WINDOW_HEIGHT * (16.0/9.0))

using namespace cv;
using namespace std;
using namespace gpu;