cmake_minimum_required(VERSION 3.14)

set(
    SRC
    exception/exception.cpp
    string/utils.cpp
    tree_value/tree_value.cpp
    tree_value/json_io.cpp
    io/binary_io.cpp
    containers/slice.cpp
    containers/utils.cpp
    memory/copy_on_move_ref.cpp
)

add_library(lib_util ${SRC})
target_include_directories(lib_util PUBLIC ${PROJECT_SOURCE_DIR})
