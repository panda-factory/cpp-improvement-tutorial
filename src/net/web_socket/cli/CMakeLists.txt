SET(WS_CLI_DIR .)

SET(WS_CLI_SRC
    ${WS_CLI_DIR}/session.cpp)

#ADD_SUBDIRECTORY(implement)

ADD_LIBRARY(ws_cli STATIC ${WS_CLI_DIR})

#TARGET_LINK_LIBRARIES(ws_cli PRIVATE http_cli_impl)