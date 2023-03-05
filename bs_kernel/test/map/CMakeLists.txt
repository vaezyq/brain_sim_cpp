project(TestGenerateMap)
enable_testing()
add_executable(TestGenerateMap generetaMapTest.cpp)

include_directories(${THIRD_PARTY_GTEST_INC_DIR}
        ${BS_INCLUDE_DIR})
IF (WIN32)
    target_link_libraries(TestGenerateMap
            ${GTEST_LIB_WINDOWS}
            ${GTEST_MAIN_LIB_WINDOWS}
            )
ELSEIF(UNIX)
    target_link_libraries(TestGenerateMap
            ${GTEST_LIB_LINUX}
            ${GTEST_MAIN_LIB_LINUX}
            )
ENDIF()
