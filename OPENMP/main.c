#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "GOL_OPENMP.h"


int main(int argc, char **argv)
{
	MPI_Request **sendingArray, **receivingArray;
	MPI_Comm CartesianCommunicator;
    //user defined datatypes for easier partial block manipulation
    MPI_Datatype MPI_Row, MPI_Column;
    //local arrays storing the state of pixels in the current iteration
	char **currentBlock;
	char **previousBlock;
	int *neighbourIDs;
	int localRows, localColumns, rows, columns, iterations, processID, totalProcs, cartesianAxis, mpi_status;
	double start, end, localDuration, totalDuration;

    if(argc != 7)
    {
        printf("Wrong number of arguments. Proper form of input : -rw [rows] -cl [columns] -it [iterations]\n");
        exit(0);
    }

    getArguments(argc, argv, &rows, &columns, &iterations);
    //initializing the MPI thread functions
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &mpi_status);
    //getting the number of processes
	MPI_Comm_size(MPI_COMM_WORLD,&totalProcs);
    //getting the identificator of current process
	MPI_Comm_rank(MPI_COMM_WORLD, &processID);
    //calculating the axis of the cartesian topology and the size of the local block
    calculateLocalVars(processID, rows, columns, totalProcs, &cartesianAxis, &localRows, &localColumns);
    //checking whether given dimensions allow for proper block distribution
    //improper dimensions lead to program termination
    checkDimensions(rows, columns, cartesianAxis);
    //initializing the 2D cartesian communicator
    getCartesianCommunicator(cartesianAxis, &CartesianCommunicator);
    //initializing useful datatypes for process communication
    initMpiDatatypes(&MPI_Row, &MPI_Column, localRows, localColumns);
    //allocating memory for process' data block
    initializeLocalPlains(&localRows, &localColumns, &previousBlock, &currentBlock);
    //initialize pixels of the block
    initializeEnvironment(localRows, localColumns, previousBlock);
    //getting current process' rank in the cartesian communicator
    MPI_Comm_rank(CartesianCommunicator, &processID);
    //getting the ids of the neighbouring processes
    getNeighboursID(processID, &neighbourIDs, CartesianCommunicator);
    //initializing communication arrays containing request ids for send/receive with each neighbour
    initializeCommArrays(&sendingArray, &receivingArray, 2, 8);
    //initializing communication between each process and its neighbours
    initializeCommunication(neighbourIDs, localColumns, localRows, previousBlock, currentBlock, MPI_Row, MPI_Column, CartesianCommunicator, sendingArray, receivingArray);
    MPI_Barrier(CartesianCommunicator);

    start = MPI_Wtime();
    GOL_Simulation(iterations, localRows, localColumns, &previousBlock, &currentBlock, sendingArray, receivingArray, CartesianCommunicator);
    end = MPI_Wtime();

    localDuration = end - start;

    //equating the total time spent to one sum printed by the main process
    MPI_Reduce(&localDuration, &totalDuration, 1, MPI_DOUBLE, MPI_MAX, 0, CartesianCommunicator);
    if(processID == 0)
    {
        printf("Total duration is equal to %f seconds\n", totalDuration);
    }

    //freeing MPI resources
    MPI_Comm_free(&CartesianCommunicator);
    MPI_Type_free(&MPI_Row);
    MPI_Type_free(&MPI_Column);
	MPI_Finalize();

    //freeing arrays
    free(neighbourIDs);
    free(sendingArray);
    free(receivingArray);
	free(previousBlock);
	free(currentBlock);

	exit(0);
}
