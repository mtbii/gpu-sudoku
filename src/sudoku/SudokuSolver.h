#pragma once

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <fstream>
#include <cstring>


#include <cuda_runtime.h>
#include <algorithm>
#include <curand.h>
#include <vector>

#include "CudaSudoku_cuda.cuh"

using namespace std;

class SudokuSolver
{
public:
	SudokuSolver();
	virtual ~SudokuSolver();
	vector<vector<int>> solve(vector<vector<int>> puzzle);

private:

};

