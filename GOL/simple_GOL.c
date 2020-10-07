#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "simple_GOL.h"

//memory allocation for the subsequent plains
void initializeArrays(char** currPlain, char** prevPlain, int rows, int columns){
  int i;

  currPlain = malloc(sizeof(char *) * rows);
  prevPlain = malloc(sizeof(char *) * rows);

  for(i = 0; i < rows; i++){
    currPlain[i] = malloc(sizeof(char) * columns);
    prevPlain[i] = malloc(sizeof(char) * columns);
  }

  return;
}

//setting a state for each pixel of the given plain
void initializeEnvironment(char** givenPlain, int rows, int columns){
  int i, j;

  for(i = 0; i < rows; i++){
    for(j = 0; j < columns; j++){
      //deciding randomly whether pixel is alive or dead
      //alive prob = 1/3, dead = 2/3
      givenPlain[i][j] = (rand() % 3) == 0;
    }
  }
  return;
}

void produceNextPlain(char** prevPlain, char** currPlain, int rows, int columns){
  int i, j, neighbours;

  for(i = 0; i < rows; i++){
    for(j = 0; j < columns; j++){
      neighbours = getNumOfNeighbours(prevPlain, i, j, rows, columns);
      //evaluating the state for the (i,j) pixel in the new plain
      currPlain[i][j] = updateState(prevPlain, i, j, neighbours);
    }
  }
  return;
}

//current plain is becoming the previous one for the next iteration
void swapPlains(char** prevPlain, char** currPlain, char** suppPlain){
  suppPlain = prevPlain;
  prevPlain = currPlain;
  currPlain = suppPlain;

  return;
}

//we are using the Moore Neighbour definition,
//each pixel has 8 neighbours, even the ones at the corner
int getNumOfNeighbours(char** prevPlain, int xAxis, int yAxis, int rows, int columns){
  //<c>Top = c coordinate bigger than the central point's equivalent
  //<c>Bottom = c coordinate smaller than the central point's equivalent
  int i, j, xTop, xBottom, yTop, yBottom, totalNeighbours;

  xTop = (xAxis + rows - 1) % rows;
  xBottom = (xAxis + 1) % rows;
  yTop = (yAxis + columns - 1) % columns;
  yBottom = (yAxis + 1) % columns;

  //checking all possible tuples of coordinates
  //getting the state of the neighbours
  int x_points[] = {xTop, xAxis, xBottom};
  int y_points[] = {yTop, yAxis, yBottom};

  totalNeighbours = 0;

  for(i = 0; i < 3; i++){
    for(j = 0; j < 3; j++){
      //pixel is not a neighbour of itself
      if(i != 1 && j != 1){
        totalNeighbours += prevPlain[x_points[i]][y_points[j]];
      }
    }
  }

  return totalNeighbours;
}

int updateState(char** prevPlain, int xAxis, int yAxis, int neighbours){

  //pixes used to be dead
  if(prevPlain[xAxis][yAxis] == 0){
    //bringing the pixel back to life
    if(neighbours == 3){
      return 1;
    }else{
      return 0;
    }//pixel used to be alive
  }else{
    //has 2 or 3 neighbours, we keep it alive
    if(neighbours == 2 || neighbours == 3){
      return 1;
    }else{
      return 0;
    }
  }
}

//deallocating the memory for the two plains
void freeArrays(char** prevPlain, char** currPlain, int rows, int columns){
  int i;

  for(i = 0; i < columns; i++){
    free(prevPlain[i]);
    free(currPlain[i]);
  }
  free(prevPlain);
  free(currPlain);
  return;
}

//calculating time using millisecond to second conversion
double calculateTime(struct timeval start, struct timeval stop){
  double first_difference = (double)(stop.tv_sec - start.tv_sec);
  double second_difference = (double)(stop.tv_usec - start.tv_usec);
  return first_difference + (second_difference / 1000000);
}
