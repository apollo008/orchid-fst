cmake_minimum_required(VERSION 3.5)

set(TOP_PROJECT_NAME orchid-fst)  #need set
message(TOP_PROJECT_NAME=${TOP_PROJECT_NAME})

project(${TOP_PROJECT_NAME})

string(TOUPPER ${TOP_PROJECT_NAME}  TOP_PROJECT_NAME_UPPER)
message(TOP_PROJECT_NAME_UPPER=${TOP_PROJECT_NAME_UPPER})

set(${TOP_PROJECT_NAME_UPPER}_DEPEND_PREFIX_DIR $ENV{HOME}/local)   #need set
message("${TOP_PROJECT_NAME_UPPER}_DEPEND_PREFIX_DIR=${${TOP_PROJECT_NAME_UPPER}_DEPEND_PREFIX_DIR}")


set(${TOP_PROJECT_NAME_UPPER}_ALL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(${TOP_PROJECT_NAME_UPPER}_ALL_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
message("${TOP_PROJECT_NAME_UPPER}_ALL_SOURCE_DIR=${${TOP_PROJECT_NAME_UPPER}_ALL_SOURCE_DIR}")
message("${TOP_PROJECT_NAME_UPPER}_ALL_BINARY_DIR=${${TOP_PROJECT_NAME_UPPER}_ALL_BINARY_DIR}")


execute_process( COMMAND mkdir -p ${${TOP_PROJECT_NAME_UPPER}_DEPEND_PREFIX_DIR})
set(ENV{CC} gcc)
message("CC=$ENV{CC}")
set(ENV{CXX} g++)
message("CXX=$ENV{CXX}")

set(ENV{PATH} ${${TOP_PROJECT_NAME_UPPER}_DEPEND_PREFIX_DIR}/bin:$ENV{PATH})
message("PATH=$ENV{PATH}")
set(ENV{LD_LIBRARY_PATH} ${${TOP_PROJECT_NAME_UPPER}_DEPEND_PREFIX_DIR}/lib:$ENV{LD_LIBRARY_PATH})
message("LD_LIBRARY_PATH=$ENV{LD_LIBRARY_PATH}")

set(ENV{CMAKE_INCLUDE_PATH} ${${TOP_PROJECT_NAME_UPPER}_DEPEND_PREFIX_DIR}/include)
message("CMAKE_INCLUDE_PATH=$ENV{CMAKE_INCLUDE_PATH}")
set(ENV{CMAKE_LIBRARY_PATH} ${${TOP_PROJECT_NAME_UPPER}_DEPEND_PREFIX_DIR}/lib:${${TOP_PROJECT_NAME_UPPER}_DEPEND_PREFIX_DIR}/lib64)
message("CMAKE_LIBRARY_PATH=$ENV{CMAKE_LIBRARY_PATH}")

add_definitions(-Wall -fPIC -m64 -D_FILE_OFFSET_BITS=64 -std=c++17 -gdwarf-3 -msse4.2)
set (CMAKE_SHARED_LINKER_FLAGS "-rdynamic")
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)

enable_testing()

option(ENABLE_BUILD_SHARE "Enable Build Share directory"  OFF)

link_directories(
        ${${TOP_PROJECT_NAME_UPPER}_DEPEND_PREFIX_DIR}/lib
        ${${TOP_PROJECT_NAME_UPPER}_DEPEND_PREFIX_DIR}/lib64
)

#cmake .. -DENABLE_BUILD_SHARE=ON
if (ENABLE_BUILD_SHARE)
    include_directories(
            ${${TOP_PROJECT_NAME_UPPER}_DEPEND_PREFIX_DIR}/include
    )
    link_directories(
            ${${TOP_PROJECT_NAME_UPPER}_DEPEND_PREFIX_DIR}/lib
            ${${TOP_PROJECT_NAME_UPPER}_DEPEND_PREFIX_DIR}/lib64
    )

    add_subdirectory(share/tulip-log)
    add_subdirectory(share/cppunit)
else()
    include_directories(
            ${${TOP_PROJECT_NAME_UPPER}_DEPEND_PREFIX_DIR}/include
    )
    link_directories(
            ${${TOP_PROJECT_NAME_UPPER}_DEPEND_PREFIX_DIR}/lib
            ${${TOP_PROJECT_NAME_UPPER}_DEPEND_PREFIX_DIR}/lib64
    )
    add_subdirectory(main)
endif()








