# Game-of-Life
Implementation of the famous [Game of Life (by John Conway)](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life) , using Parallel Architectures.  
</br>

## Explanation of this Theoritical Construction:

The universe of the Game of Life is an infinite, two-dimensional orthogonal grid of square cells, each of which is in one of two possible states, live or dead, (or populated and unpopulated, respectively). Every cell interacts with its eight neighbours, which are the cells that are horizontally, vertically, or diagonally adjacent. At each step in time, the following transitions occur:

1.  Any live cell with fewer than two live neighbours dies, as if by underpopulation.
2.  Any live cell with two or three live neighbours lives on to the next generation.
3.  Any live cell with more than three live neighbours dies, as if by overpopulation.
4.  Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.

**These rules, which compare the behavior of the automaton to real life, can be condensed into the following:**

1.  Any live cell with two or three live neighbours survives.
2.  Any dead cell with three live neighbours becomes a live cell.
3.  All other live cells die in the next generation. Similarly, all other dead cells stay dead.

## Collaborating Game of Life and Parallel Programming:

The main idea was to create a `N x N` Grid, and initializing it at the start of the programm, with random (either dead or alive) cells.  
The user's input was the `rows (N)` , the `columns (N)` and the number of `iterations` of the game. 

We came fast to an interesting conclusion: When we increase either the number of the grid or either the number of iterations, the complexity of this problem **increases dramatically**.  


Here comes the utility of MPI and the help of Parallel Programming.  
More specifically, we had at our disposal a super-computer named ARGO(80 cores and 2 GPU). We wondered how much more efficient our program could be, using more resources. We experimented using max-64cores exploiting the usage of the [MPI](https://en.wikipedia.org/wiki/Message_Passing_Interface) and [OpenMP](https://en.wikipedia.org/wiki/OpenMP), and the power of the available GPU, using [CUDA](https://en.wikipedia.org/wiki/CUDA).  

## Compiling and Running MPI:

*   __GOL__: In this folder we have the serial Game of Life, in order to help us do the research and the measurements about the escalation of our program. To compile and run the program, simple, the only thing you have to do is to type: `make` , and then `./main <rows> <columns> <iterations>`.  

*   __MPI__: In this folder we have the simple MPI-Game of Life implementation. In order to compile and run the program, things here get a little complicated. First, we have to type ofcourse: `make` , and then `qsub myPBSScript.sh` . This script is given us from the start of this assigment to make our life more easier. 
In order not to go into unnecessary details, we can say just that, in this script we can "play" with the number of nodes and the number of the processes-per-node. And ofcourse, inside this script the executable program is running, with the appropriate arguments each time.  </br>
Our request to run the program gets inside a Queue. After a little while, 2 files will reproduce: __myJob.o*****__ and __myJob.e*****__ .The __myJob.o*__ file contains the final message of the time which has passed to complete the program, while in the __myJob.e*__ file, all the error messages will appear.

*   __OPENMP__ and __CUDA__: The process here is the same as previous, nothing changes.  

### Further informations

This project contains also a detailed explaination of this project.  
For this big project, I've collaborated with the fellow partner: [Jakub Maciejewski](https://www.linkedin.com/in/jakub-maciejewski-0270291b7/)

*This project is part of the course: Parallel Systems , Winter of 2019. University of Athens, DiT.*
