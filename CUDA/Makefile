# In  order  to  execute  this  "Makefile " just  type  "make "
OBJS     = main.o GOL_GPU.o GOL_CUDA.o
SOURCE   = main.cpp GOL_CUDA.c 
HEADER   = GOL_CUDA.h
OUT      = gol_cuda
CC       = g++
FLAGS    = -g -Wall -c 
# -g  option  enables  debugging  mode
# -c flag  generates  object  code  for  separate  files
all: $(OBJS)
	nvcc -g $(OBJS) -o $(OUT) 

main.o: main.cpp $(HEADER)
	$(CC) $(FLAGS) main.cpp 

GOL_GPU.o: GOL_GPU.cu
	nvcc -c GOL_GPU.cu
	   
GOL_CUDA.o: GOL_CUDA.c GOL_CUDA.h
	$(CC) $(FLAGS) GOL_CUDA.c
	
#  clean  house
clean :
	rm -f $(OBJS) $(OUT)
