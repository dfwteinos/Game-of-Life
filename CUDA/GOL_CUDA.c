#include "GOL_CUDA.h"

void getArguments(int argc, char **argv, int *rows, int *columns, int *iterations)
{
    for(int i = 0; i < argc; i++)
    {
        if(!strcmp(argv[i], "-rw"))
        {
            *rows = atoi(argv[i+1]);
        }
        else if(!strcmp(argv[i], "-cl"))
        {
            *columns = atoi(argv[i+1]);
        }
        else if(!strcmp(argv[i], "-it"))
        {
          *iterations = atoi(argv[i+1]);
        }
    }
}


void initializePlains(int rows, int columns, char ***previousPlain , char ***currentPlain)
{
    char *previousPartialPlain, *currentPartialPlain;
    //allocating contiguous memory for the local array, so we can easily
    //send and receive border rows and plains

    *previousPlain = (char**)malloc(rows * sizeof(char*) + rows * columns * sizeof(char));
    *currentPlain = (char**)malloc(rows * sizeof(char*) + rows * columns * sizeof(char));

    //pointers corresponding to contiguous rows of the array
    previousPartialPlain = (char*) (*previousPlain + rows);
    currentPartialPlain = (char*) (*currentPlain + rows);

    //storing consecutive plain rows in contiguous memory
    for(int i = 0; i < rows; i++)
    {
        (*previousPlain)[i] = previousPartialPlain + i * columns;
        (*currentPlain)[i] = currentPartialPlain + i * columns;
    }

    return;
}

void initializeEnvironment(int rows, int columns, char** givenBlock)
{
  int i, j;

  for(i = 0; i < rows; i++)
  {
    for(j = 0; j < columns; j++)
    {
      //deciding randomly whether pixel is alive or dead
      //alive prob = 1/3, dead = 2/3
      givenBlock[i][j] = ((rand() % 3) == 0);
    }
  }
  return;
}

void printPlain(int rows, int columns, char **plain)
{
  for(int i = 0; i < rows; i++)
  {
    for(int j = 0; j < columns; j++)
    {
      printf("%d", plain[i][j]);
    }
    printf("\n");
  }
  return;
}

void printVector(int rows, int columns, char *vector)
{
  int totalPixels = rows * columns;

  for(int i = 0; i < totalPixels; i++)
  {
    if(i % columns == 0)
    {
      printf("\n");
    }
    printf("%d", vector[i]);
  }
  printf("\n");
}