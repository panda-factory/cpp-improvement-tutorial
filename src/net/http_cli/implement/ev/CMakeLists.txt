add_compile_options("/std:c++17")

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${HTTP_CLI_DIR})

SET(HTTP_CLI_IMPL_SRC
    host_check.cpp
    openssl_hostname_validation.cpp
    session_impl_ev.cpp)

ADD_LIBRARY(http_cli_impl STATIC ${HTTP_CLI_IMPL_SRC})

TARGET_LINK_DIRECTORIES(http_cli_impl PRIVATE ${PROJECT_SOURCE_DIR}/third_party/libevent/lib)
TARGET_LINK_LIBRARIES(http_cli_impl PRIVATE http_cli ${OPENSSL_LIBRARIES} Crypt32.lib)