project(TestMpi)
enable_testing()
add_executable(TestMpi mpiTest.cpp)
add_executable(TestMpiEnv mpiRunEnv.cpp)
add_executable(TestMpiSpeed mpiSendSpeedTest.cpp)

include_directories(${THIRD_PARTY_GTEST_INC_DIR}
        ${BS_INCLUDE_DIR} ${THIRD_PARTY_MPI_INC_DIR})

#find_package(MPI REQUIRED)
#target_link_libraries(TestMpi PUBLIC MPI::MPI_CXX)
IF (WIN32)
    target_link_libraries(TestMpi
            ${GTEST_LIB_WINDOWS}
            ${GTEST_MAIN_LIB_WINDOWS}
            MPI::MPI_CXX
            pthread)

ELSEIF (UNIX)
    target_link_libraries(TestMpi
            ${GTEST_LIB_LINUX}
            ${GTEST_MAIN_LIB_LINUX}
            MPI::MPI_CXX
            pthread)

ENDIF ()





