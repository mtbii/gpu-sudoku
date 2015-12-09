#include "SudokuSolver.h"


SudokuSolver::SudokuSolver()
{
}


SudokuSolver::~SudokuSolver()
{
}

vector<vector<int>> SudokuSolver::solve(vector<vector<int>> puzzle){

	const unsigned int threadsPerBlock = 512;
	const unsigned int maxBlocks = 1024;
	int *board = new int[N*N];

	for (int i = 0; i < puzzle.size(); i++){
		for (int j = 0; j < puzzle[0].size(); j++){
			board[i*puzzle[0].size() + j] = puzzle[j][i];
		}
	}

	// the boards after the next iteration of breadth first search
	int *new_boards;
	// the previous boards, which formthe frontier of the breadth first search
	int *old_boards;
	// stores the location of the empty spaces in the boards
	int *empty_spaces;
	// stores the number of empty spaces in each board
	int *empty_space_count;
	// where to store the next new board generated
	int *board_index;

	// maximum number of boards from breadth first search
	const int sk = pow(2, 26);

	// allocate memory on the device
	cudaMalloc(&empty_spaces, sk * sizeof(int));
	cudaMalloc(&empty_space_count, (sk / 81 + 1) * sizeof(int));
	cudaMalloc(&new_boards, sk * sizeof(int));
	cudaMalloc(&old_boards, sk * sizeof(int));
	cudaMalloc(&board_index, sizeof(int));

	// same as board index, except we need to set board_index to zero every time and this can stay
	int total_boards = 1;

	// initialize memory
	cudaMemset(board_index, 0, sizeof(int));
	cudaMemset(new_boards, 0, sk * sizeof(int));
	cudaMemset(old_boards, 0, sk * sizeof(int));

	// copy the initial board to the old boards
	cudaMemcpy(old_boards, board, N * N * sizeof(int), cudaMemcpyHostToDevice);

	// call the kernel to generate boards
	callBFSKernel(maxBlocks, threadsPerBlock, old_boards, new_boards, total_boards, board_index,
		empty_spaces, empty_space_count);

	// number of boards after a call to BFS
	int host_count;
	// number of iterations to run BFS for
	int iterations = 18;

	// loop through BFS iterations to generate more boards deeper in the tree
	for (int i = 0; i < iterations; i++) {
		cudaMemcpy(&host_count, board_index, sizeof(int), cudaMemcpyDeviceToHost);

		printf("total boards after an iteration %d: %d\n", i, host_count);

		cudaMemset(board_index, 0, sizeof(int));


		if (i % 2 == 0) {
			callBFSKernel(maxBlocks, threadsPerBlock, new_boards, old_boards, host_count, board_index, empty_spaces, empty_space_count);
		}
		else {
			callBFSKernel(maxBlocks, threadsPerBlock, old_boards, new_boards, host_count, board_index, empty_spaces, empty_space_count);
		}
	}

	cudaMemcpy(&host_count, board_index, sizeof(int), cudaMemcpyDeviceToHost);
	printf("new number of boards retrieved is %d\n", host_count);

	// flag to determine when a solution has been found
	int *dev_finished;
	// output to store solved board in
	int *dev_solved;

	// allocate memory on the device
	cudaMalloc(&dev_finished, sizeof(int));
	cudaMalloc(&dev_solved, N * N * sizeof(int));

	// initialize memory
	cudaMemset(dev_finished, 0, sizeof(int));
	cudaMemcpy(dev_solved, board, N * N * sizeof(int), cudaMemcpyHostToDevice);

	if (iterations % 2 == 1) {
		// if odd number of iterations run, then send it old boards not new boards;
		new_boards = old_boards;
	}

	cudaSudokuBacktrack(maxBlocks, threadsPerBlock, new_boards, host_count, empty_spaces,
		empty_space_count, dev_finished, dev_solved);


	// copy back the solved board
	int *solved = new int[N * N];

	memset(solved, 0, N * N * sizeof(int));

	cudaMemcpy(solved, dev_solved, N * N * sizeof(int), cudaMemcpyDeviceToHost);

	//printBoard(solved);

	vector<vector<int>> solvedPuzzle = vector<vector<int>>(N, vector<int>(N, 0));

	for (int i = 0; i < solvedPuzzle.size(); i++){
		for (int j = 0; j < solvedPuzzle[0].size(); j++){
			solvedPuzzle[i][j] = solved[i*puzzle[0].size() + j];
		}
	}

	// free memory
	delete[] board;
	delete[] solved;

	cudaFree(empty_spaces);
	cudaFree(empty_space_count);
	cudaFree(new_boards);
	cudaFree(old_boards);
	cudaFree(board_index);

	cudaFree(dev_finished);
	cudaFree(dev_solved);

	return solvedPuzzle;
}