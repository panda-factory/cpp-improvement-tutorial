SET(CMAKE_CXX_STANDARD 17)
SET(HTTP_CLI_TEST_DIR ../../../src/net/http/cli)

INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)

SET(HTTP_CLI_TEST_SRC
    http_cli_unittests.cpp
    ${HTTP_CLI_TEST_DIR}/session_unittests.cpp
    )

ADD_EXECUTABLE(http_cli_unittests ${HTTP_CLI_TEST_SRC})

TARGET_LINK_DIRECTORIES(http_cli_unittests PRIVATE ${PROJECT_SOURCE_DIR}/third_party/libevent/lib)
TARGET_LINK_LIBRARIES(http_cli_unittests PRIVATE ${GTEST_LIBRARIES} core http_cli ${OPENSSL_LIBRARIES} event_openssl.lib event_extra.lib event_core.lib ws2_32.lib winmm.lib wldap32.lib)