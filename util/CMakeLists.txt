cmake_minimum_required(VERSION 3.12)

set(
    SRC
    type/utils.cpp
    type/type_list.cpp
    type/list_utils.cpp
    exception/exception.cpp
    memory/reserve_t.cpp
    memory/move_on_rvalue_ptr.cpp
    string/utils.cpp
    tree_value/tree_value.cpp
    tree_value/json_io.cpp
    io/binary_io.cpp
    container/slice.cpp
    container/utils.cpp
)

add_library(lib_util ${SRC})
target_include_directories(lib_util PUBLIC ${PROJECT_SOURCE_DIR})
