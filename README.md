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
