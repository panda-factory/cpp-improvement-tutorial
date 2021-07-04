
SET(GTEST_ROOT ${PROJECT_SOURCE_DIR}/third_party/googletest-1.10.0/googletest)

FIND_PACKAGE(GTestSrc REQUIRED)

IF(GTESTSRC_FOUND)
    enable_testing()

    IF (WIN32 AND (NOT CYGWIN) AND (NOT MINGW))
        SET(gtest_disable_pthreads ON)
        SET(gtest_force_shared_crt ON)
    ENDIF()


    ADD_SUBDIRECTORY(${GTEST_SOURCE_DIR} ${CMAKE_BINARY_DIR}/googletest)
    #include_directories(SYSTEM ${GTEST_INCLUDE_DIR})
    SET(GTEST_LIBRARIES gtest gtest_main)

    #add_custom_target(tests ALL)
    #ADD_SUBDIRECTORY(net/http_cli)
    #ADD_SUBDIRECTORY(net/ws_cli)
    #ADD_SUBDIRECTORY(pipeline)
    ADD_SUBDIRECTORY(wtf/sync)
    ADD_SUBDIRECTORY(wtf/task_runner)
    #add_subdirectory(unittest)

    INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)
    ADD_EXECUTABLE(unittests unittests.cpp)
    TARGET_LINK_LIBRARIES(unittests PRIVATE ${GTEST_LIBRARIES})
ENDIF(GTESTSRC_FOUND)
