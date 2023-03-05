cmake_minimum_required(VERSION 3.21)
project(brain_sim)

IF (WIN32)
    MESSAGE(STATUS "Now is windows")
ELSEIF (APPLE)
    MESSAGE(STATUS "Now is Apple systens.")
ELSEIF (UNIX)
    MESSAGE(STATUS "Now is UNIX-like OS's.")
ENDIF ()

set(CMAKE_CXX_STANDARD 17)

set (PROJECT_KERNEL_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
MESSAGE(${CMAKE_CURRENT_SOURCE_DIR})

set (BS_INCLUDE_DIR ${PROJECT_KERNEL_BASE_DIR}/bs_kernel/inc)
set (THIRD_PARTY_BASE_DIR ${PROJECT_KERNEL_BASE_DIR}/3rd_party)

# set google test directory
set (THIRD_PARTY_GTEST_INC_DIR ${THIRD_PARTY_BASE_DIR}/googletest-1.12.1/googletest/include)
set (THIRD_PARTY_GTEST_LIB_DIR ${THIRD_PARTY_BASE_DIR}/googletest-1.12.1/googletest/libs)
set (THIRD_PARTY_GMOCK_INC_DIR ${THIRD_PARTY_BASE_DIR}/googletest-1.12.1/googlemock/include)
set (THIRD_PARTY_GMOCK_LIB_DIR ${THIRD_PARTY_BASE_DIR}/googletest-1.12.1/googlemock/libs)

# set google test library libs name
set (GTEST_LIB_LINUX ${THIRD_PARTY_GTEST_LIB_DIR}/linux/libgtest.a)
set (GTEST_MAIN_LIB_LINUX ${THIRD_PARTY_GTEST_LIB_DIR}/linux/libgtest_main.a)
set (GTEST_LIB_WINDOWS ${THIRD_PARTY_GTEST_LIB_DIR}/windows/gtest.lib)
set (GTEST_MAIN_LIB_WINDOWS ${THIRD_PARTY_GTEST_LIB_DIR}/windows/gtest_main.lib)

# set rapidjson directory
set (THIRD_PARTY_RAPID_JSON_INC_DIR ${THIRD_PARTY_BASE_DIR}/rapidjson)

# set rapidjson directory
set (THIRD_PARTY_MPI_INC_DIR ${THIRD_PARTY_BASE_DIR}/mpich-x86_64)



include_directories("/usr/include/mpich-x86_64")
add_executable(generateDefaultRoute test/generateRoute.cpp code/Utils/AssertUtils.hpp code/Utils/ProcessFileDataUtils.hpp code/Utils/GenerateRouteUtils.hpp)


add_executable(loadDataTest test/loadDataTest.cpp)


add_executable(simulationTrafficTest code/LoadData.hpp code/BaseInfo.hpp code/Utils/Utils.hpp test/trafficSimulationTest.cpp code/MpiManage.hpp code/Utils/SimulationTrafficUtils.hpp code/SimulationOneDimTraffic.hpp code/SimulationHighDimTraffic.hpp)


add_executable(randomTest test/testRandom.cpp)

add_executable(mpiTest test/mpiTest.cpp)

add_executable(someTest test/someTest.cpp code/GenerateMapParallel.hpp code/MapSplitMethod.hpp)

add_executable(generateMap code/LoadData.hpp code/BaseInfo.hpp code/Utils/Utils.hpp test/generateMap.cpp code/MpiManage.hpp code/Utils/SimulationTrafficUtils.hpp code/MapSplitMethod.hpp code/Utils/SimTrafficForGenMapUtils.hpp code/Utils/GenerateMapUtils.hpp)


find_package(MPI REQUIRED)

target_link_libraries(mpiTest PUBLIC MPI::MPI_CXX)

target_link_libraries(simulationTrafficTest PUBLIC MPI::MPI_CXX)

target_link_libraries(generateMap PUBLIC MPI::MPI_CXX)

add_subdirectory(bs_kernel)
