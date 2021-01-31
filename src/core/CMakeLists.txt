
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)
SET(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

SET(CORE_SRC
    ${CORE_DIR}/log_setting.cpp
    ${CORE_DIR}/log_settings_state.cpp
    ${CORE_DIR}/logging.cpp)

ADD_LIBRARY(core STATIC ${CORE_SRC})

