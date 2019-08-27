cmake_minimum_required(VERSION 3.14)

set(
    SRC
    exception/exception.cpp
    string/utils.cpp
    tree_value/tree_value.cpp
    io/json_io.cpp
    io/binary_io.cpp
)

add_library(lib_util ${SRC})
target_include_directories(lib_util PUBLIC ${PROJECT_SOURCE_DIR})
