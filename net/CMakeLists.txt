cmake_minimum_required(VERSION 3.12)

set(
    SRC
    http/message.cpp
)

add_library(lib_net ${SRC})
target_link_libraries(lib_net lib_util)
add_dependencies(lib_net lib_util)
target_include_directories(lib_net PUBLIC ${PROJECT_SOURCE_DIR})