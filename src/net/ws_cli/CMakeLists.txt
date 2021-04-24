SET(WS_CLI_DIR .)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)

if (USE_LIBEVENT)
    INCLUDE_DIRECTORIES(${LIBEVENT_INCLUDE_DIR})
elseif(USE_LIBUV)
    INCLUDE_DIRECTORIES(${LIBUV_INCLUDE_DIR})
    INCLUDE_DIRECTORIES(${LIBHTTP_PARSER_INCLUDE_DIR})
endif ()

SET(WS_CLI_SRC
    ${WS_CLI_DIR}/connect_handler.cpp
    ${WS_CLI_DIR}/response.cpp
    ${WS_CLI_DIR}/message_handler.cpp
    ${WS_CLI_DIR}/session.cpp
    ${WS_CLI_DIR}/session_impl.cpp
    ${WS_CLI_DIR}/util.cpp
    ${WS_CLI_DIR}/ws_header.cpp
    ${WS_CLI_DIR}/../request.cpp
    ${WS_CLI_DIR}/../url.cpp)

ADD_SUBDIRECTORY(implement)

ADD_LIBRARY(ws_cli STATIC ${WS_CLI_SRC})

TARGET_LINK_LIBRARIES(ws_cli PRIVATE ws_cli_impl)