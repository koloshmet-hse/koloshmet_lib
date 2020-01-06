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
    opt/opt.cpp
    opt/options.cpp
)

add_library(lib_util ${SRC})
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0.0)
        target_link_libraries(lib_util c++fs)
    endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_link_libraries(lib_util stdc++fs)
endif()

target_include_directories(lib_util PUBLIC ${PROJECT_SOURCE_DIR})
