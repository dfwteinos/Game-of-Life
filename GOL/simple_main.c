#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "simple_GOL.h"

//main takes three arguments (rows, columns, iterations)
int main(int argc, char **argv){
  //we need three plains for updates
  char** currPlain;
  char** prevPlain;
  char ** suppPlain;
  int i, j, iter, columns, rows, iterations;
  double totalTime;
  struct timeval stop, start;
  srand(time(NULL));

  //user didn't give us 3 arguments
  if(argc != 4){
    printf("Input error, wrong number of arguments!\n");
    return -1;
  }else{
    rows = atoi(argv[1]);
    columns = atoi(argv[2]);
    iterations = atoi(argv[3]);
  }

  //our plain must be NxN and with positive coordinates
  if(rows != columns){
    printf("Given dimensions are not NxN!\n");
    return -1;
  }else if(rows < 0 || columns < 0){
    printf("Given dimensions are negative!\n");
    return -1;
  }

  //allocating memory for the subsequent plains
  initializeArrays(currPlain, prevPlain, rows, columns);
  //initializing previous plain in order to feed info to the current one
  initializeEnvironment(prevPlain, rows, columns);

  gettimeofday(&start, NULL);

  //repeating the given amount of iterations of the game
  for(iter = 0; iter < iterations; iter++){
    //for each pixel of previous plain, updating the current one
    produceNextPlain(prevPlain, currPlain, rows, columns);
    swapPlains(prevPlain, currPlain, suppPlain);
  }

  gettimeofday(&stop, NULL);
  printf("Total time spent: %lf\n", calculateTime(start, stop));
  freeArrays(prevPlain, currPlain, rows, columns);
  return 0;
}
