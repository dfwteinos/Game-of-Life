#some name types for readable make file
OB = main.o GOL_OPENMP.o
HEADERS = GOL_OPENMP.h
EXEC = gol_openmp
COMP = mpicc
FLAGS = -g -Wall -c 
#executable
all: gol_openmp

gol_openmp: $(OB)
	$(COMP) -g $(OB) -o $(EXEC) -lm -fopenmp

main.o: main.c $(HEADERS)
	$(COMP) $(FLAGS) main.c 
	   
GOL_OPENMP.o: GOL_OPENMP.c GOL_OPENMP.h
	$(COMP) $(FLAGS) GOL_OPENMP.c -fopenmp
	
#  cleaning command
clean :
	rm -f $(OB) $(EXEC)
