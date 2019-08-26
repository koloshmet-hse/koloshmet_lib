cmake_minimum_required(VERSION 3.14)
project(koloshmet_lib)

set(CMAKE_CXX_STANDARD 17)

if (DEFINED APPLE)
    link_directories(/usr/local/opt/llvm/lib)
endif ()

add_subdirectory(util)
add_subdirectory(posix)
