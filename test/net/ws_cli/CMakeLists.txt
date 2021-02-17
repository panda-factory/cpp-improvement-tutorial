
SET(WS_CLI_TEST_DIR ${PROJECT_SOURCE_DIR}/src/net/ws_cli)

INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)

SET(WS_CLI_TEST_SRC
    ws_cli_unittests.cpp
    ${WS_CLI_TEST_DIR}/session_unittests.cpp
    ${WS_CLI_TEST_DIR}/implement/ev/base64_unittests.cpp
    )

ADD_EXECUTABLE(ws_cli_unittests ${WS_CLI_TEST_SRC})

TARGET_LINK_DIRECTORIES(ws_cli_unittests PRIVATE ${PROJECT_SOURCE_DIR}/third_party/libevent/lib)
TARGET_LINK_LIBRARIES(ws_cli_unittests PRIVATE ${GTEST_LIBRARIES} core ws_cli ${OPENSSL_LIBRARIES} event_openssl.lib event_extra.lib event_core.lib ws2_32.lib winmm.lib wldap32.lib)