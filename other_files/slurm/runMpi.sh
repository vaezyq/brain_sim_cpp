#!/bin/bash
mpicxx mpiTest.cpp -o mpiTest
mpirun -np 8 ./mpiTest
