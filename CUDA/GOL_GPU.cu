#include "GOL_CUDA.h"
#include <cuda_runtime_api.h>
#include "lcutil.h"
#include "timestamp.h"

__global__ void GOL_Simulation(int rows, int columns, int plainSize, char *prevPlain, char *currPlain)
{
    const unsigned int thread_pos = blockIdx.x * blockDim.x + threadIdx.x;
    int totalNeighbours, upperNeighbours, sideNeighbours, lowerNeighbours;
    //total number of pixels up to the row over the current point
    //and the number of pixels up to the current row
    //simplifying the expressions for neighbouring positions
    int upperPlainPixels, pixelsUpToRow;
    //the y coordinates on the left and right of the current point
    //keeping in mind our plain is interconnected
    int leftY, rightY;
    //coordinates of current point
    int currX, currY;

    //taking upper bound number of blocks
    //some threads will be obsolete
    if(thread_pos < plainSize)
    {
        currX = thread_pos / columns;
        currY = thread_pos - currX * columns;

        leftY = (currY + columns - 1) % columns;
        rightY = (currY + 1) % columns;

        upperPlainPixels = columns * ((currX + rows - 1) % rows);
        pixelsUpToRow = columns * ((currX + 1) % rows);

        //calculating the neighbours
        upperNeighbours = prevPlain[upperPlainPixels + leftY] + prevPlain[upperPlainPixels + currY] + prevPlain[upperPlainPixels + rightY];
        sideNeighbours = prevPlain[currX * columns + leftY] + prevPlain[currX * columns + rightY];
        lowerNeighbours = prevPlain[pixelsUpToRow + leftY] + prevPlain[pixelsUpToRow + currY] + prevPlain[pixelsUpToRow + rightY];
        //add the three groups of neighbours
        totalNeighbours = upperNeighbours + sideNeighbours + lowerNeighbours;

        //pixel survives
        if(totalNeighbours == 3 || (totalNeighbours == 2 && prevPlain[thread_pos] == 1))
        {
            currPlain[thread_pos] = 1;
        }else//pixel dies
        {
            currPlain[thread_pos] = 0;
        }
    }
}


//function used to print error messages for cuda errors
int checkFunction(cudaError_t functionOutput, const char *msg)
{
    if (functionOutput != cudaSuccess)
    {
        perror(msg);
        exit(functionOutput);
    }
    return 0;
}


extern "C" float GPU_GOL(int rows, int columns, int iterations, char **prevPlain, char **currPlain)
{
    int totalPixels = rows * columns;
    int plainMemory = totalPixels * sizeof(char);
    int iteration;
    double microseconds;
    //plains stored in gpu memory
    char *d_prevPlain, *d_currPlain, *d_tmpPlain;
    //block and threads variables for CUDA code
    const int THREADS_PER_BLOCK = 1024;
    dim3 blockNum(THREADS_PER_BLOCK);
    dim3 threadNum(FRACTION_CEILING(totalPixels, THREADS_PER_BLOCK));

    //allocating memory for the GPU plains and copying data from CPU to GPU
    checkFunction(cudaMalloc((void**) &d_prevPlain, plainMemory), "Failed to allocate memory for GPU previous plain");
    checkFunction(cudaMemcpy(d_prevPlain, &(prevPlain[0][0]), plainMemory, cudaMemcpyHostToDevice), "Failed to copy plain from CPU to GPU");
    checkFunction(cudaMalloc((void**) &d_currPlain, plainMemory), "Failed to allocate memory for GPU previous plain");

    timestamp t_start;
    t_start = getTimestamp();
    //actual simulation execution
    for(iteration = 0; iteration < iterations; iteration++)
    {
        GOL_Simulation<<<threadNum, blockNum>>>(rows, columns, totalPixels, d_prevPlain, d_currPlain);
        //checking whether an error occured during current GOL simulation's iteration
        checkFunction(cudaGetLastError(), "Error occured during GPU execution");
        //current plain is the previous updated one
        d_tmpPlain = d_prevPlain;
        d_prevPlain = d_currPlain;
        d_currPlain = d_tmpPlain;
    }
    cudaDeviceSynchronize();
    microseconds = getElapsedtime(t_start);
    //copy the final plain from gpu to cpu memory
    checkFunction(cudaMemcpy(&(currPlain[0][0]), d_currPlain, plainMemory, cudaMemcpyDeviceToHost), "Failed to copy plain from GPU to CPU");
    //freeing the memory allocated in the gpu
    checkFunction(cudaFree(d_prevPlain), "Failed to free GPU previous plain");
    checkFunction(cudaFree(d_currPlain), "Failed to free GPU current plain");
    
    return microseconds;
}