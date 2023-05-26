project(TestGenerateRoute)
enable_testing()
add_executable(TestGenerateRoute generateDefaultRoute.cpp generateRouteByTraffic.hpp)

include_directories(${THIRD_PARTY_GTEST_INC_DIR}
        ${BS_INCLUDE_DIR})
IF (WIN32)
    target_link_libraries(TestGenerateRoute
            ${GTEST_LIB_WINDOWS}
            ${GTEST_MAIN_LIB_WINDOWS}
            )
ELSEIF(UNIX)
    target_link_libraries(TestGenerateRoute
            ${GTEST_LIB_LINUX}
            ${GTEST_MAIN_LIB_LINUX}
            )
ENDIF()
