project(common)

include_directories(
        ${${TOP_PROJECT_NAME_UPPER}_ALL_SOURCE_DIR}/main/common
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}  # for test.h
)

set(DOTEST_CPP ${CMAKE_CURRENT_SOURCE_DIR}/common/test/dotest.cpp)

add_subdirectory(common)
