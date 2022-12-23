cmake_minimum_required(VERSION 3.21)
project(brain_sim)

set(CMAKE_CXX_STANDARD 17)

include_directories("/usr/include/mpich-x86_64")
add_executable(generateDefaultRoute test/generateRoute.cpp)


add_executable(loadDataTest test/loadDataTest.cpp)


add_executable(simulationTrafficTest code/LoadData.hpp code/BaseInfo.hpp code/Utils.hpp test/trafficSimulationTest.cpp code/MpiManage.hpp code/SimulationTrafficUtils.hpp code/SimulationOneDimTraffic.hpp code/SimulationHighDimTraffic.hpp)


add_executable(randomTest test/testRandom.cpp)

add_executable(mpiTest test/mpiTest.cpp)

add_executable(someTest test/someTest.cpp code/GenerateMapParallel.hpp code/MapSplitMethod.hpp)

add_executable(generateMap code/LoadData.hpp code/BaseInfo.hpp code/Utils.hpp test/generateMap.cpp code/MpiManage.hpp code/SimulationTrafficUtils.hpp code/MapSplitMethod.hpp code/SimTrafficForGenMapUtils.hpp)


find_package(MPI REQUIRED)

target_link_libraries(mpiTest PUBLIC MPI::MPI_CXX)

target_link_libraries(simulationTrafficTest PUBLIC MPI::MPI_CXX)

target_link_libraries(generateMap PUBLIC MPI::MPI_CXX)