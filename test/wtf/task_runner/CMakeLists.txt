
SET(TASK_RUNNER_TEST_DIR ${PROJECT_SOURCE_DIR}/src/wtf/task_runner)

INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/wtf)

SET(TASK_RUNNER_TEST_SRC
    ${TASK_RUNNER_TEST_DIR}/message_loop_unittests.cpp
    ${TASK_RUNNER_TEST_DIR}/thread_local_unittests.cpp
    ${TASK_RUNNER_TEST_DIR}/platform/win/message_loop_win_unittests.cpp
    )

ADD_EXECUTABLE(task_runner_unittests ${TASK_RUNNER_TEST_SRC})

TARGET_LINK_LIBRARIES(task_runner_unittests PRIVATE ${GTEST_LIBRARIES} taskrunner)
