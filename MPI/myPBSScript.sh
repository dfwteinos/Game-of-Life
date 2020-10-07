#!/bin/bash

# Max VM size #
#PBS -l pvmem=2G

# Max Wall time, Example 1 Minute #
#PBS -l walltime=00:01:00

# How many nodes and tasks per node, Example 2 nodes with 8 tasks each => 16 tasts #
#PBS -l nodes=8:ppn=8

# Which Queue to use, DO NOT CHANGE #
#PBS -q workq

# JobName #
#PBS -N myJob

#Change Working directory to SUBMIT directory
cd $PBS_O_WORKDIR

#OpenMP Threads #
# OMP_NUM_THREADS * ppn should be max 8 (the total number of node cores= 8 ).
# To use OpenMPI remember to include -fopenmp in compiler flags in order to activate OpenMP directives.
export OMP_NUM_THREADS=1

# Run executable #
mpiexec -n 64 ./gol_mpi -rw 6720 -cl 6720 -it 50
