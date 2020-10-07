#include "GOL_OPENMP.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

char* neigPos[] = {"LEFTUP", "UP", "RIGHTUP", "LEFT", "RIGHT", "LEFTBOTTOM", "BOTTOM", "RIGHTBOTTOM"};


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

void calculateLocalVars(int processID, int rows, int columns, int totalProcs, int *cartesianAxis, int *localRows, int *localColumns)
{
    //initializing the random number generator of each process based on its id
    //avoiding identical generators for processes that will start simultaneously
    srand(time(NULL) * processID);

    //the axis of the 2D cartesian topology is the square root
    //of the number of processes, simplifying mathematical expressions
    //for executions where the number of processes is a square root
    //of a natural number
    *cartesianAxis = sqrt(totalProcs);

    //calculating the area of the local block
    *localRows = rows/(*cartesianAxis);
    *localColumns = columns/(*cartesianAxis);
    return;
}

void checkDimensions(int rows, int columns, int cartesianAxis)
{
    //checking if given plain can be seperated into equal parts for each process
    if(rows % cartesianAxis != 0 || columns % cartesianAxis != 0)
    {
        printf("Given axis sizes don't allow proper block distribution!\n");
        //calling mpi finalization before exiting the whole program
        MPI_Finalize();
        exit(0);
    }
}

void initializeLocalPlains(int *localRows, int *localColumns, char ***previousPlain , char ***currentPlain)
{
    char *previousPartialPlain, *currentPartialPlain;
    int rows, columns;
    //allocating contiguous memory for the local array, so we can easily
    //send and receive border rows and plains

    //rows and columns incremented by 2 to include border pixels
    *localRows = *localRows + 2;
    *localColumns = *localColumns + 2;

    rows = *localRows;
    columns = *localColumns;

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

//returns an array containing the ids of the neighbouring processes
void getNeighboursID(int processID, int **neighbourIDs, MPI_Comm CartesianCommunicator){
  //coordinates of current process and temporary neighbour
  int processCoordinates[2];
  int currentCoordinates[2];

  //allocating memory for the neighbour IDs
  *neighbourIDs = (int*) malloc(sizeof(int) * 8);

  //getting the coordinates of current process
  MPI_Cart_coords(CartesianCommunicator, processID, 2, processCoordinates);
  int process_x = processCoordinates[0];
  int process_y = processCoordinates[1];
  //arrays containing all combinations of neighbours' coordinates
  int x_axis[] = {process_x - 1, process_x, process_x + 1};
  int y_axis[] = {process_y - 1, process_y, process_y + 1};
  int neighbourID;
  int ith = 0;

  //getting the id of each neighbour
  for(int curr_x = x_axis[0]; curr_x <= x_axis[2]; curr_x++){
    for(int curr_y = y_axis[0]; curr_y <= y_axis[2]; curr_y++){
      if(! (isCurrentProcess(curr_x, curr_y, processCoordinates))) {
        currentCoordinates[0] = curr_x;
        currentCoordinates[1] = curr_y;
        MPI_Cart_rank(CartesianCommunicator, currentCoordinates, &neighbourID);
        (*neighbourIDs)[ith] = neighbourID;
        //going to the next neighbour
        ith++;
      }
    }
  }
  return;
}

//initializing arrays containing block parts for communication between each
//process and its neighbours (2 x 8 arrays)
void initializeCommArrays(MPI_Request ***sendingArray, MPI_Request ***receivingArray, int numOfBlocks, int numOfNeighbours){
  MPI_Request *firstBlockRequests;
  MPI_Request *secondBlockRequests;

  *sendingArray = (MPI_Request**) malloc(sizeof(MPI_Request*) * numOfBlocks);
  *receivingArray = (MPI_Request**) malloc(sizeof(MPI_Request*) * numOfBlocks);

  firstBlockRequests = (MPI_Request*) malloc(sizeof(MPI_Request) * numOfNeighbours * numOfBlocks);
  secondBlockRequests = (MPI_Request*) malloc(sizeof(MPI_Request) * numOfNeighbours * numOfBlocks);

  for(int i = 0; i < numOfBlocks; i++){
    (*sendingArray)[i] = firstBlockRequests + i * numOfNeighbours;
    (*receivingArray)[i] = secondBlockRequests + i * numOfNeighbours;
  }
  return;
}


//general function for initialization of receiving and sending data from/to neighbours
void initializeCommunication(int *neighbourIDs, int columnsPerProc, int rowsPerProc, char **previousBlock, char **currentBlock, MPI_Datatype MPI_Row, MPI_Datatype MPI_Column, MPI_Comm CartesianCommunicator, MPI_Request **sendingArray, MPI_Request **receivingArray){
  int neighbour;

  for( neighbour = 0; neighbour < 8; neighbour++){
    //initializing request array for the previous block
    //array contains request ids for each neighbour
    initializeSending(neighbour, neighbourIDs[neighbour], columnsPerProc, rowsPerProc, previousBlock, MPI_Row, MPI_Column, CartesianCommunicator, &sendingArray[1][neighbour]);
    initializeReceiving(neighbour, neighbourIDs[neighbour], columnsPerProc, rowsPerProc, previousBlock, MPI_Row, MPI_Column, CartesianCommunicator, &receivingArray[1][neighbour]);

    //initializing request array for the current block
    //array contains request ids for each neighbour
    initializeSending(neighbour, neighbourIDs[neighbour], columnsPerProc, rowsPerProc, currentBlock, MPI_Row, MPI_Column, CartesianCommunicator, &sendingArray[0][neighbour]);
    initializeReceiving(neighbour, neighbourIDs[neighbour], columnsPerProc, rowsPerProc, currentBlock, MPI_Row, MPI_Column, CartesianCommunicator, &receivingArray[0][neighbour]);
  }
  return;
}

//initialize sending block parts to neighbouring processes
void initializeSending(int neighbour, int neighbourID, int columnsPerProc, int rowsPerProc, char **procBlock, MPI_Datatype MPI_Row, MPI_Datatype MPI_Column, MPI_Comm CartesianCommunicator, MPI_Request *requestID){
  /*const*/ void *topLeftCorner = (/*const*/ void *)&(procBlock[1][1]);
  /*const*/ void *topRightCorner =  (/*const*/ void *)&(procBlock[1][columnsPerProc - 2]);
  /*const*/ void *bottomLeftCorner = (/*const*/ void *)&(procBlock[rowsPerProc - 2][1]);
  /*const*/ void *bottomRightCorner = (/*const*/ void *)&(procBlock[rowsPerProc - 2][columnsPerProc - 2]);
  int communicationTag;

  if( !(strcmp("LEFTUP",neigPos[neighbour])) ){
    //sending data only from the top left halo point
    getCommunicationTag(neigPos[neighbour], &communicationTag, false);
    MPI_Send_init(topLeftCorner, 1, MPI_CHAR, neighbourID, communicationTag, CartesianCommunicator, requestID);
  }else if( !(strcmp("UP",neigPos[neighbour])) ){
    //sending data from the upper row
    getCommunicationTag(neigPos[neighbour], &communicationTag, false);
    MPI_Send_init(topLeftCorner, 1, MPI_Row, neighbourID, communicationTag, CartesianCommunicator, requestID);
  }else if( !(strcmp("RIGHTUP",neigPos[neighbour])) ){
    //sending data only from the top right halo point
    getCommunicationTag(neigPos[neighbour], &communicationTag, false);
    MPI_Send_init(topRightCorner, 1, MPI_CHAR, neighbourID, communicationTag, CartesianCommunicator, requestID);
  }else if( !(strcmp("LEFT",neigPos[neighbour])) ){
    //sending data from the left column
    getCommunicationTag(neigPos[neighbour], &communicationTag, false);
    MPI_Send_init(topLeftCorner, 1, MPI_Column, neighbourID, communicationTag, CartesianCommunicator, requestID);
  }else if( !(strcmp("RIGHT",neigPos[neighbour])) ){
    //sending data from the right column
    getCommunicationTag(neigPos[neighbour], &communicationTag, false);
    MPI_Send_init(topRightCorner, 1, MPI_Column, neighbourID, communicationTag, CartesianCommunicator, requestID);
  }else if( !(strcmp("LEFTBOTTOM",neigPos[neighbour])) ){
    //sending data from the bottom left halo point
    getCommunicationTag(neigPos[neighbour], &communicationTag, false);
    MPI_Send_init(bottomLeftCorner, 1, MPI_CHAR, neighbourID, communicationTag, CartesianCommunicator, requestID);
  }else if( !(strcmp("BOTTOM",neigPos[neighbour])) ){
    //sending data from the bottom row
    getCommunicationTag(neigPos[neighbour], &communicationTag, false);
    MPI_Send_init(bottomLeftCorner, 1, MPI_Row, neighbourID, communicationTag, CartesianCommunicator, requestID);
  }else if( !(strcmp("RIGHTBOTTOM",neigPos[neighbour])) ){
    //sending data from the left bottom halo point
    getCommunicationTag(neigPos[neighbour], &communicationTag, false);
    MPI_Send_init(bottomRightCorner, 1, MPI_CHAR, neighbourID, communicationTag, CartesianCommunicator, requestID);
  }
}

//initialize receiving block parts from neighbouring processes
void initializeReceiving(int neighbour, int neighbourID, int columnsPerProc, int rowsPerProc, char **procBlock, MPI_Datatype MPI_Row, MPI_Datatype MPI_Column, MPI_Comm CartesianCommunicator, MPI_Request *requestID){
  //pointers to the buffers of received data
  /*const*/ void *topLeftCorner = (/*const*/ void *)&(procBlock[0][0]);
  /*const*/ void *topRightCorner =  (/*const*/ void *)&(procBlock[0][columnsPerProc - 1]);
  /*const*/ void *bottomLeftCorner = (/*const*/ void *)&(procBlock[rowsPerProc - 1][0]);
  /*const*/ void *bottomRightCorner = (/*const*/ void *)&(procBlock[rowsPerProc - 1][columnsPerProc - 1]);
  /*const*/ void *upperRow = (/*const*/ void *)&(procBlock[0][1]);
  /*const*/ void *leftColumn = (/*const*/ void *)&(procBlock[1][0]);
  /*const*/ void *rightColumn = (/*const*/ void *)&(procBlock[1][columnsPerProc - 1]);
  /*const*/ void *bottomRow = (/*const*/ void *)&(procBlock[rowsPerProc - 1][1]);

  int communicationTag;

  if( !(strcmp("LEFTUP",neigPos[neighbour])) ){
    //sending data only from the top left halo point
    getCommunicationTag(neigPos[neighbour], &communicationTag, true);
    MPI_Recv_init(topLeftCorner, 1, MPI_CHAR, neighbourID, communicationTag, CartesianCommunicator, requestID);
  }else if( !(strcmp("UP",neigPos[neighbour])) ){
    //sending data from the upper row
    getCommunicationTag(neigPos[neighbour], &communicationTag, true);
    MPI_Recv_init(upperRow, 1, MPI_Row, neighbourID, communicationTag, CartesianCommunicator, requestID);
  }else if( !(strcmp("RIGHTUP",neigPos[neighbour])) ){
    //sending data only from the top right halo point
    getCommunicationTag(neigPos[neighbour], &communicationTag, true);
    MPI_Recv_init(topRightCorner, 1, MPI_CHAR, neighbourID, communicationTag, CartesianCommunicator, requestID);
  }else if( !(strcmp("LEFT",neigPos[neighbour])) ){
    //sending data from the left column
    getCommunicationTag(neigPos[neighbour], &communicationTag, true);
    MPI_Recv_init(leftColumn, 1, MPI_Column, neighbourID, communicationTag, CartesianCommunicator, requestID);
  }else if( !(strcmp("RIGHT",neigPos[neighbour])) ){
    //sending data from the right column
    getCommunicationTag(neigPos[neighbour], &communicationTag, true);
    MPI_Recv_init(rightColumn, 1, MPI_Column, neighbourID, communicationTag, CartesianCommunicator, requestID);
  }else if( !(strcmp("LEFTBOTTOM",neigPos[neighbour])) ){
    //sending data from the bottom left halo point
    getCommunicationTag(neigPos[neighbour], &communicationTag, true);
    MPI_Recv_init(bottomLeftCorner, 1, MPI_CHAR, neighbourID, communicationTag, CartesianCommunicator, requestID);
  }else if( !(strcmp("BOTTOM",neigPos[neighbour])) ){
    //sending data from the bottom row
    getCommunicationTag(neigPos[neighbour], &communicationTag, true);
    MPI_Recv_init(bottomRow, 1, MPI_Row, neighbourID, communicationTag, CartesianCommunicator, requestID);
  }else if( !(strcmp("RIGHTBOTTOM",neigPos[neighbour])) ){
    //sending data from the left bottom halo point
    getCommunicationTag(neigPos[neighbour], &communicationTag, true);
    MPI_Recv_init(bottomRightCorner, 1, MPI_CHAR, neighbourID, communicationTag, CartesianCommunicator, requestID);
  }
}

//getting communication tag for the neighbouring process
//described by the given string
void getCommunicationTag(char *neighbourName, int *communicationTag, bool receiving){
  int localTag;

  if( !(strcmp("LEFTUP", neighbourName)) ) {
    localTag = 7;
  }else if( !(strcmp("UP", neighbourName)) ){
    localTag = 6;
  }else if( !(strcmp("RIGHTUP", neighbourName)) ){
    localTag = 5;
  }else if( !(strcmp("LEFT", neighbourName)) ){
    localTag = 4;
  }else if( !(strcmp("RIGHT", neighbourName)) ){
    localTag = 3;
  }else if( !(strcmp("LEFTBOTTOM", neighbourName)) ){
    localTag = 2;
  }else if( !(strcmp("BOTTOM", neighbourName)) ){
    localTag = 1;
  }else if( !(strcmp("RIGHTBOTTOM", neighbourName)) ){
    localTag = 0;
  }

  //when receiving a message, it will be stores in the part of the array
  //from where the request was casted
  if(receiving)
  {
    *communicationTag = 7 - localTag;
  }else//sending the request to the opposite process
  {
    *communicationTag = localTag;
  }
  return;
}

//function simulates the game of life for given number of iterations
void GOL_Simulation(int iterations, int rowsPerProc, int columnsPerProc, char ***previousBlock, char ***currentBlock, MPI_Request **sendingArray, MPI_Request **receivingArray, MPI_Comm CartesianCommunicator)
{
  int requestRow;
  int localDifference;
  int difference;
  int iteration;
  char **temporaryBlock;

  //loop variables for the traversal of the inner plain for each thread
  int threadRow, threadColumn;
  int active_neighbours;

  #pragma omp parallel private (iteration, requestRow, active_neighbours, threadRow, threadColumn)
  {
    for(iteration = 0; iteration < iterations; iteration++)
    {
      //which row of request array should we traverse?
      calculateCommRow(iteration, &requestRow);
      //executing the send/receive requests
      //mpi actions must be carried out by the master process
      //execution of requests is one of them
      #pragma omp master
      {
        executeRequests(requestRow, 8, receivingArray);
      }
      #pragma omp master
      {
        executeRequests(requestRow, 8, sendingArray);
      }

      // #pragma omp parallel for collapse(2)
      for( threadRow = 2; threadRow < rowsPerProc - 2; threadRow++)
      {
        #pragma omp for
        for( threadColumn = 2; threadColumn < columnsPerProc - 2; threadColumn++)
        {
          plainGetNeighbours(threadRow, threadColumn, &active_neighbours, *previousBlock);
          updateState(threadRow, threadColumn, active_neighbours, previousBlock, currentBlock);
        }
      }

      //updateInternalBlock(rowsPerProc, columnsPerProc, &threadRow, &threadColumn, &active_neighbours, previousBlock, currentBlock);

      #pragma omp master
      {
        updateBoundsOfBlock(8, rowsPerProc, columnsPerProc, requestRow, previousBlock, currentBlock, receivingArray);
      }

      //updating the corners must be thread safe, only on of them can change the memory of the corners
      #pragma omp single
      {
        updateCorners(rowsPerProc, columnsPerProc, previousBlock, currentBlock);
      }
      //master thread takes care of the mpi related sending requests execution
      #pragma omp master
      {
        terminateSending(requestRow, 8, sendingArray);
      }
      //current block is becoming the previous one, next iteration
      if(iteration % 10 == 0)
      {
        //checking if the two consecutive plains have changed, are not the same
        plainsDifferent(rowsPerProc, columnsPerProc, &localDifference, *previousBlock, *currentBlock);
        MPI_Allreduce(&localDifference, &difference, 1, MPI_INT, MPI_LOR, CartesianCommunicator);

        //the two plains are not different, but we won't stop the execution
        if(!difference)
        {
          printf("%d\n", iteration);
          break;
        }
      }

      //we have to make sure that swapping the consequent plains is thread safe
      //waiting for all of them to come to this point
      #pragma omp barrier
      //simple plain update
      #pragma omp single
      {
        temporaryBlock = *previousBlock;
        *previousBlock = *currentBlock;
        *currentBlock = temporaryBlock;
      }
    }
  }
  return;
}

//checks if plain has changed during current iteration
void plainsDifferent(int rows, int columns, int *different, char **prevPlain, char **currPlain)
{
  int i, j;

  for(i = 0; i < rows; i++)
  {
    for(j = 0; j < columns; j++)
    {
      //found two different pixels, plain has changed
      if(prevPlain[i][j] != currPlain[i][j])
      {
        *different = 1;
        return;
      }
    }
  }
  *different = 0;
  return;
}

//dictates whether we will use the first or second row of request array
//starting with second one
void calculateCommRow(int currIteration, int *requestRow)
{
  *requestRow = (currIteration % 2);
  return;
}

//executing requests concerning each of the neighbours
void executeRequests(int requestRow, int numOfNeighbours, MPI_Request **requestArray)
{
  int neighbour;

  for( neighbour = 0; neighbour < numOfNeighbours; neighbour++)
  {
    MPI_Start(&(requestArray[requestRow][neighbour]));
  }
  return;
}

//updating the internal pixels of process' block
void updateInternalBlock(int rowsPerProc, int columnsPerProc, int *active_neighbours, char ***previousBlock, char ***currentBlock)
{
  int curr_row, curr_column;
  //updating only the internal pixels in range [2 - > axis - 2]
  for( curr_row = 2; curr_row < rowsPerProc - 2; curr_row++)
  {
    #pragma omp for
    for( curr_column = 2; curr_column < columnsPerProc - 2; curr_column++)
    {
      plainGetNeighbours(curr_row, curr_column, active_neighbours, *previousBlock);
      updateState(curr_row, curr_column, *active_neighbours, previousBlock, currentBlock);
    }
  }
  return;
}

//getting the number of active neighbouring pixels
//traversing internal part of block, no need for border check
void plainGetNeighbours(int x_point, int y_point, int *active_neighbours, char **previousBlock)
{
  int x_axis, y_axis;
  int x_min = x_point - 1;
  int x_max = x_point + 1;
  int y_min = y_point - 1;
  int y_max = y_point + 1;

  int neighbours = 0;

  for( x_axis = x_min; x_axis <= x_max; x_axis++ )
  {
    for( y_axis = y_min; y_axis <= y_max; y_axis++)
    {
      //current point is not neighbour of itself!
      if(!(x_axis == x_point && y_axis == y_point))
      {
        //neighbour is active
        if(previousBlock[x_axis][y_axis] == 1)
        {
          neighbours++;
        }
      }
    }
  }
  *active_neighbours = neighbours;
  return;
}

//setting the state of the pixel corresponding to given coordinates
void updateState(int x_axis, int y_axis, int active_neighbours, char ***previousBlock, char ***currentBlock)
{

  //pixes used to be dead
  if((*previousBlock)[x_axis][y_axis] == 0)
  {
    //bringing the pixel back to life
    if(active_neighbours == 3)
    {
      (*currentBlock)[x_axis][y_axis] = 1;
    }else
    {
      (*currentBlock)[x_axis][y_axis] = 0;
    }//pixel used to be alive
  }else
  {
    //has 2 or 3 neighbours, we keep it alive
    if(active_neighbours == 2 || active_neighbours == 3)
    {
      (*currentBlock)[x_axis][y_axis] = 1;
    }else
    {
      (*currentBlock)[x_axis][y_axis] = 0;
    }
  }
  return;
}

void updateBoundsOfBlock(int numOfNeighbours, int rowsPerProc, int columnsPerProc, int requestRow, char ***previousBlock, char ***currentBlock, MPI_Request **receivingArray)
{
  int neighbour, indx, row, column, active_neighbours;
  MPI_Status status;
  //row containing request ids for all neighbouring processes
  MPI_Request *receiveRow = receivingArray[requestRow];

  for( neighbour = 0; neighbour < numOfNeighbours; neighbour++ )
  {
    //waiting for one of the non-diagonal neighbours
    //so we can receive data, necessary for boundary update
    MPI_Waitany(numOfNeighbours, receiveRow, &indx, &status);

    //updating upper row
    if(! (strcmp("UP",neigPos[neighbour] )))
    {

      for( column = 2;  column < columnsPerProc - 2; column++ )
      {
        plainGetNeighbours(1, column, &active_neighbours, *previousBlock);
        updateState(1, column, active_neighbours, previousBlock, currentBlock);
      }
    //updating left column
    }else if(!(strcmp("LEFT",neigPos[neighbour] )))
    {

      for( row = 2;  row < rowsPerProc - 2; row++ )
      {
        plainGetNeighbours(row, 1, &active_neighbours, *previousBlock);
        updateState(row, 1, active_neighbours, previousBlock, currentBlock);
      }
    //updating right column
    }else if(!(strcmp("RIGHT",neigPos[neighbour])))
    {

      for( row = 2;  row < rowsPerProc - 2; row++ )
      {
        plainGetNeighbours(row, columnsPerProc - 2, &active_neighbours, *previousBlock);
        updateState(row, columnsPerProc - 2, active_neighbours, previousBlock, currentBlock);
      }
    //updating bottom row
    }else if(!(strcmp("BOTTOM",neigPos[neighbour])))
    {

      for( column = 2;  column < columnsPerProc - 2; column++ )
      {
        plainGetNeighbours(rowsPerProc - 2, column, &active_neighbours, *previousBlock);
        updateState(rowsPerProc - 2, column, active_neighbours, previousBlock, currentBlock);
      }
    }
    return;
  }
}

//data up to data from internal block and neighbour bound
//can finally calculate corner pixels
void updateCorners(int rowsPerProc, int columnsPerProc, char ***previousBlock, char ***currentBlock)
{
  int active_neighbours;

  //updating left top corner
  plainGetNeighbours(1, 1, &active_neighbours, *previousBlock);
  updateState(1, 1, active_neighbours, previousBlock, currentBlock);

  //updating right top corner
  plainGetNeighbours(1, columnsPerProc - 2, &active_neighbours, *previousBlock);
  updateState(1, columnsPerProc - 2, active_neighbours, previousBlock, currentBlock);

  //updating left bottom corner
  plainGetNeighbours(rowsPerProc - 2, 1, &active_neighbours, *previousBlock);
  updateState(rowsPerProc - 2, 1, active_neighbours, previousBlock, currentBlock);

  //updating right bottom corner
  plainGetNeighbours(rowsPerProc - 2, columnsPerProc - 2, &active_neighbours, *previousBlock);
  updateState(rowsPerProc - 2, columnsPerProc - 2, active_neighbours, previousBlock, currentBlock);
  return;
}

//waiting till all the data is sent to neighbouring processes
void terminateSending(int requestRow, int numOfNeighbours, MPI_Request **sendingArray)
{
  int neighbour;
  MPI_Status status;

  //each neighbour has to receive our request
  for(neighbour = 0; neighbour < numOfNeighbours; neighbour++)
  {
    MPI_Wait(&(sendingArray[requestRow][neighbour]) , &status);
  }
  return;
}

//setting a state for each pixel of the given block
void initializeEnvironment(int localRows, int localColumns, char** givenBlock)
{
  int i, j;

  for(i = 0; i < localRows; i++)
  {
    for(j = 0; j < localColumns; j++)
    {
      //deciding randomly whether pixel is alive or dead
      //alive prob = 1/3, dead = 2/3
      givenBlock[i][j] = ((rand() % 3) == 0);
    }
  }
  return;
}

void getCartesianCommunicator(int cartesianAxis, MPI_Comm *CartesianCommunicator){
  int totalDimensions, reorder;
  int dimensions[] = {cartesianAxis, cartesianAxis};
  int periods[] = {1 , 1};

  //two dimensional cartesian topology
  totalDimensions = 2;
  //allowing process id reordering
  reorder = 1;
  //calling MPI cartesian topology communicator constructor
  MPI_Cart_create(MPI_COMM_WORLD, totalDimensions, dimensions, periods, reorder, CartesianCommunicator);
  return;
}

//useful for data parts exchange between processes
void initMpiDatatypes(MPI_Datatype *MPI_Row, MPI_Datatype *MPI_Column, int localRows, int localColumns){

  //columns store two more integers representing the halo points
  MPI_Type_contiguous(localColumns, MPI_CHAR, MPI_Row); 
  MPI_Type_commit(MPI_Row);
  MPI_Type_vector(localRows, 1, localColumns + 2, MPI_CHAR, MPI_Column);
  MPI_Type_commit(MPI_Column);
  return;
}

int isCurrentProcess(int curr_x, int curr_y, int *processCoordinates)
{
  return ((curr_x == processCoordinates[0]) && (curr_y == processCoordinates[1]));
}


