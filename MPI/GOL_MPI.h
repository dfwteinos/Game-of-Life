#ifndef GOL_MPI
#define GOL_MPI

#include "mpi.h"
#include <stdbool.h>

void getArguments(int, char**, int*, int*, int*);
void calculateLocalVars(int, int, int, int, int*, int*, int*);
void checkDimensions(int, int, int);
void initializeLocalPlains(int*,int*, char ***, char ***);
void initializeEnvironment(int, int, char**);
void getNeighboursID(int, int**, MPI_Comm);
int isCurrentProcess(int, int, int*);
void initializeCommArrays(MPI_Request***, MPI_Request***, int, int);
void initializeCommunication(int*, int, int, char**, char**, MPI_Datatype, MPI_Datatype, MPI_Comm, MPI_Request**, MPI_Request**);
void initializeSending(int, int, int, int, char**, MPI_Datatype, MPI_Datatype, MPI_Comm, MPI_Request*);
void initializeReceiving(int, int, int, int, char**, MPI_Datatype, MPI_Datatype, MPI_Comm, MPI_Request*);
void getCommunicationTag(char*, int*, bool);
void GOL_Simulation(int, int, int, char***, char***, MPI_Request**, MPI_Request**, MPI_Comm);
void plainsDifferent(int, int, int*, char**, char**);
void calculateCommRow(int, int*);
void executeRequests(int, int, MPI_Request**);
void updateInternalBlock(int, int, char***, char***);
void plainGetNeighbours(int, int, int*, char**);
void updateState(int, int, int, char***, char***);
void updateBoundsOfBlock(int, int, int, int, char***, char***, MPI_Request**);
void updateCorners(int, int, char***, char***);
void terminateSending(int, int, MPI_Request**);
void getCartesianCommunicator(int, MPI_Comm*);
void initMpiDatatypes(MPI_Datatype*, MPI_Datatype*, int, int);

#endif
