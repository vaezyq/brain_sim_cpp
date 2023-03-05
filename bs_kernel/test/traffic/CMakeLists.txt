project(TestTraffic)
enable_testing()
add_executable(TestTraffic trafficSimulationTest.cpp)
find_package(MPI REQUIRED)
include_directories(${THIRD_PARTY_GTEST_INC_DIR}
        ${BS_INCLUDE_DIR})
IF (WIN32)
    target_link_libraries(TestTraffic
            ${GTEST_LIB_WINDOWS}
            ${GTEST_MAIN_LIB_WINDOWS}
            ${MPI_CXX}
            )
ELSEIF (UNIX)
    target_link_libraries(TestTraffic
            ${GTEST_LIB_LINUX}
            ${GTEST_MAIN_LIB_LINUX}
            ${MPI_CXX}
            )
ENDIF ()

#target_link_libraries(TestTraffic PUBLIC MPI::MPI_CXX)
