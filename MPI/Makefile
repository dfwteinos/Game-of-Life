#some name types for readable make file
OB = main.o GOL_MPI.o
HEADERS = GOL_MPI.h
EXEC = gol_mpi
COMP = mpicc
FLAGS = -g -Wall -c 
#executable
all: gol_mpi

gol_mpi: $(OB)
	$(COMP) -g $(OB) -o $(EXEC) -lm

main.o: main.c $(HEADERS)
	$(COMP) $(FLAGS) main.c 
	   
GOL_MPI.o: GOL_MPI.c GOL_MPI.h
	$(COMP) $(FLAGS) GOL_MPI.c
	
#  cleaning command
clean :
	rm -f $(OB) $(EXEC)
