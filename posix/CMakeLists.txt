cmake_minimum_required(VERSION 3.14)

set(
    SRC
    file_descriptor/fd.cpp
    file_descriptor/unique_fd.cpp
    file_descriptor/fd_stream.cpp
)

add_library(lib_posix ${SRC})
target_include_directories(lib_posix PUBLIC ${PROJECT_SOURCE_DIR})
