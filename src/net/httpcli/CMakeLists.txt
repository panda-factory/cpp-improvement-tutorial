
ADD_SUBDIRECTORY(implement/beast)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)

SET(HTTP_CLI_DIR ${CMAKE_CURRENT_SOURCE_DIR})

SET(HTTP_CLI_SRC
    ${HTTP_CLI_DIR}/Authentication.cpp
    ${HTTP_CLI_DIR}/Body.cpp
    ${HTTP_CLI_DIR}/Cookies.cpp
    ${HTTP_CLI_DIR}/Digest.cpp
    ${HTTP_CLI_DIR}/Handler.cpp
    ${HTTP_CLI_DIR}/Header.cpp
    ${HTTP_CLI_DIR}/MaxRedirects.cpp
    ${HTTP_CLI_DIR}/Method.cpp
    ${HTTP_CLI_DIR}/Multipart.cpp
    ${HTTP_CLI_DIR}/Parameters.cpp
    ${HTTP_CLI_DIR}/Session.cpp
    ${HTTP_CLI_DIR}/SessionImpl.cpp
    ${HTTP_CLI_DIR}/Timeout.cpp
    ${HTTP_CLI_DIR}/Url.cpp)

ADD_LIBRARY(httpcli STATIC ${HTTP_CLI_SRC})
