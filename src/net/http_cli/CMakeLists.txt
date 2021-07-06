
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)
if (USE_LIBEVENT)
    INCLUDE_DIRECTORIES(${LIBEVENT_INCLUDE_DIR})
elseif(USE_LIBUV)
    INCLUDE_DIRECTORIES(${LIBUV_INCLUDE_DIR})
    INCLUDE_DIRECTORIES(${LIBHTTP_PARSER_INCLUDE_DIR})
endif ()

if (ENABLE_HTTPS)
    INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
endif ()

SET(HTTP_CLI_DIR ${CMAKE_CURRENT_SOURCE_DIR})

SET(HTTP_CLI_SRC
    ${HTTP_CLI_DIR}/authentication.cpp
    ${HTTP_CLI_DIR}/body.cpp
    ${HTTP_CLI_DIR}/cookies.cpp
    ${HTTP_CLI_DIR}/digest.cpp
    ${HTTP_CLI_DIR}/error.cpp
    ${HTTP_CLI_DIR}/maxRedirects.cpp
    ${HTTP_CLI_DIR}/multipart.cpp
    ${HTTP_CLI_DIR}/parameters.cpp
    ${HTTP_CLI_DIR}/response.cpp
    ${HTTP_CLI_DIR}/session.cpp
    ${HTTP_CLI_DIR}/session_impl.cpp
    ${HTTP_CLI_DIR}/timeout.cpp
    ${HTTP_CLI_DIR}/../request.cpp
    ${HTTP_CLI_DIR}/../url.cpp)

ADD_SUBDIRECTORY(implement)

ADD_LIBRARY(http_cli STATIC ${HTTP_CLI_SRC})

TARGET_LINK_LIBRARIES(http_cli PRIVATE http_cli_impl logging)

