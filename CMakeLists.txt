cmake_minimum_required(VERSION 3.12)
project(koloshmet_lib)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra")

if (DEFINED APPLE AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    link_directories(/usr/local/opt/llvm/lib)
endif ()

add_subdirectory(util)
add_subdirectory(posix)
