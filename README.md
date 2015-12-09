# gpu-sudoku
Solving a Sudoku puzzle using computer vision and CUDA.  
Final project for University of South Carolina Parallel Computing class, CSCE 569.

#### Technnologies
- OpenCV 2.4.12
- CUDA (Toolkit 7.5, Compute 5.0)

#### Implementation
OpenCV is used for extraction of the Sudoku puzzle from a video feed. The main.h file specifies has a define statement that allows the use the GPU for performing the machine vision algorithms including thresholding and edge detection.

After thresholding and filtering, the digit images are extracted and recognized using OpenCV's KNearest class. This uses a K nearest neighbor algorithm with the images in the "train" folder as the models for comparison. The dataset is small and made for printed text and should work with simple computer-generated texts.

The GPU is always used to solve the puzzle although the method is not as efficient as some basic CPU algorithms.

#### Testing
This code was run on a Dell Alienware 15 with a Intel Core i7-4710HQ CPU and Nvidia Geforce GTX 970M GPU. When running this on a different architecture, you will be required to build OpenCV from source and specify the proper architecture for CUDA in CMake (in my case the "CUDA_ARCH_BIN" value had to be "2.0 3.0 3.5 5.0" and the "CUDA_ARCH_PTX" value had to be "5.0" to get OpenCV to interface with the GPU).

#### Results
Extraction:
CPU: ~1200 ms  
GPU: ~2100 ms  

Solving:
CPU: < 2 ms  
GPU: ~200 ms  
