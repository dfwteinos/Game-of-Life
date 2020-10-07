#include "GOL_CUDA.h"

extern "C" float GPU_GOL(int , int, int, char**, char**);

int main(int argc, char **argv)
{
    //pointers to plains allocated in cpu memory
    char **prevPlain, **currPlain;
    int rows, columns, iterations;
    float microseconds;
    //allocating memory and initializing the plain in the cpu memory
    getArguments(argc, argv, &rows, &columns, &iterations);
    initializePlains(rows , columns, &prevPlain, &currPlain);
    initializeEnvironment(rows, columns, prevPlain);
    //executing game of life on gpu
    microseconds = GPU_GOL(rows, columns, iterations, prevPlain, currPlain);
    printf("Execution time %.3f msecs\n", microseconds);
    //C++ program frees the cpu related data
    free(prevPlain);
	free(currPlain);
    return 0;
}