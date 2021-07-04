
SET(LOGGING_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/log_setting.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/log_settings_state.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/logging.cpp)

ADD_LIBRARY(logging STATIC ${LOGGING_SRC})
