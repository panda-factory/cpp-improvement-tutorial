
SET(BOOST_ROOT "${PROJECT_SOURCE_DIR}/third_party/boost_1_75_0")

FIND_PACKAGE(Boost COMPONENTS COMPONENTS
    # regex
    filesystem   # 我的工程中只使用了 boost 的 filesystem 功能,因此这里只有一个组件
    )

IF(NOT Boost_FOUND)
    MESSAGE("Not found Boost with root: " + ${BOOST_ROOT})
ENDIF()