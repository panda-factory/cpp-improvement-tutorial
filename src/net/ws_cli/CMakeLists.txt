SET(WS_CLI_DIR .)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${LIBEVENT_INCLUDE_DIR})

SET(WS_CLI_SRC
    ${WS_CLI_DIR}/connect_handler.cpp
    ${WS_CLI_DIR}/message_handler.cpp
    ${WS_CLI_DIR}/session.cpp
    ${WS_CLI_DIR}/session_impl.cpp)

ADD_SUBDIRECTORY(implement)

ADD_LIBRARY(ws_cli STATIC ${WS_CLI_SRC})

TARGET_LINK_LIBRARIES(ws_cli PRIVATE ws_cli_impl)