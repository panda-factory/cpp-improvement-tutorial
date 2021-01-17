# For boost
SET(BOOST_ROOT "${PROJECT_SOURCE_DIR}/third_party/boost_1_74_0")

FIND_PACKAGE(Boost COMPONENTS COMPONENTS)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

IF(NOT Boost_FOUND)
    MESSAGE("Not found Boost with root: " + ${BOOST_ROOT})
ENDIF()

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)
SET(HTTP_CLI_DIR ${CMAKE_CURRENT_SOURCE_DIR})

SET(HTTP_CLI_SRC
    ${HTTP_CLI_DIR}/authentication.cpp
    ${HTTP_CLI_DIR}/body.cpp
    ${HTTP_CLI_DIR}/cookies.cpp
    ${HTTP_CLI_DIR}/digest.cpp
    ${HTTP_CLI_DIR}/handler.cpp
    ${HTTP_CLI_DIR}/maxRedirects.cpp
    ${HTTP_CLI_DIR}/method.cpp
    ${HTTP_CLI_DIR}/multipart.cpp
    ${HTTP_CLI_DIR}/parameters.cpp
    ${HTTP_CLI_DIR}/session.cpp
    ${HTTP_CLI_DIR}/session_impl.cpp
    ${HTTP_CLI_DIR}/timeout.cpp
    ${HTTP_CLI_DIR}/url.cpp)

SET(USE_HTTP_BEAST 1)
ADD_SUBDIRECTORY(implement)

ADD_LIBRARY(http_cli STATIC ${HTTP_CLI_SRC})

target_link_libraries(http_cli PRIVATE http_cli_impl)
