#ifndef CUDA_SUDOKU_CUDA_CUH
#define CUDA_SUDOKU_CUDA_CUH


#define N 9
#define n 3



void cudaSudokuBacktrack(const unsigned int blocks,
        const unsigned int threadsPerBlock,
        int *boards,
        const int numBoards,
        int *emptySpaces,
        int *numEmptySpaces,
        int *finished,
        int *solved);

void callBFSKernel(const unsigned int blocks, 
                        const unsigned int threadsPerBlock,
                        int *old_boards,
                        int *new_boards,
                        int total_boards,
                        int *board_index,
                        int *empty_spaces,
                        int *empty_space_count); 
#endif // CUDA_SUDOKU_CUDA_CUH
