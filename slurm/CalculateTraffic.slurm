#!/bin/bash
#SBATCH --job=wml_calculate_traffic_test
#SBATCH --partition=kshdexclu04
#SBATCH --exclude=./black_list
#SBATCH --mem=90G
#SBATCH --exclusive
#SBATCH --ntasks-per-node=8
## SBATCH --cpus-per-task=8

#SBATCH -N 126
#SBATCH --gres=dcu:4

export UCX_TLS=sm,rc_x

date

module rm compiler/rocm/3.9.1
module add compiler/cmake/3.15.6
module add compiler/rocm/4.0.1

# mpirun -np 201 python mapAnalysis.py
mpicxx -std=c++17 ../test/trafficSimulationTest.cpp ../code/SimulationTraffic.hpp ../code/LoadData.hpp ../code/TimePrint.hpp ../code/BaseInfo.hpp -o trafficSimulation -lstdc++fs
mpirun --bind-to none --mca pml ucx --mca osc ucx -np 1001 ./trafficSimulation
date
