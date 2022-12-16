cmake_minimum_required(VERSION 3.21)
project(brain_sim)

set(CMAKE_CXX_STANDARD 17)

include_directories("/usr/include/mpich-x86_64")
add_executable(generateDefaultRoute test/generateRoute.cpp)


add_executable(loadDataTest test/loadDataTest.cpp)


add_executable(simulationTrafficTest test/trafficSimulationTest.cpp code/mapSplitMethod.hpp code/SimulationHighDimTraffic.hpp code/MpiManage.hpp code/SimulationTrafficUtils.hpp)


add_executable(randomTest test/testRandom.cpp)

add_executable(someTest test/someTest.cpp)

find_package(MPI REQUIRED)

target_link_libraries(simulationTrafficTest PUBLIC MPI::MPI_CXX)