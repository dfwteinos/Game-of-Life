#ifndef GOL_CUDA
#define GOL_CUDA

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void getArguments(int , char **, int *, int *, int *);
void initializePlains(int, int, char ***, char ***);
void initializeEnvironment(int, int, char**);
void printPlain(int, int, char**);
void printVector(int, int, char *);


#endif