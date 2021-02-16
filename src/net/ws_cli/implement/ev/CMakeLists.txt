
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${WS_CLI_DIR})
INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})

SET(WS_CLI_IMPL_SRC
    session_impl_ev.cpp
    util.cpp
    utf8_utils.cpp
    ws_header.cpp)

ADD_LIBRARY(ws_cli_impl STATIC ${WS_CLI_IMPL_SRC})

TARGET_LINK_DIRECTORIES(ws_cli_impl PRIVATE ../../../../../third_party/libevent/lib)
TARGET_LINK_LIBRARIES(ws_cli_impl PRIVATE ws_cli ${OPENSSL_LIBRARIES} Crypt32.lib)