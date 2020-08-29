cmake_minimum_required(VERSION 3.12)
project(koloshmet_lib)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-Werror -Wall -Wextra)

option(K_UTIL_ONLY "Build only util" ON)

add_subdirectory(util)
if(NOT K_UTIL_ONLY)
    add_subdirectory(posix)
    add_subdirectory(net)
    add_subdirectory(async)
endif()
