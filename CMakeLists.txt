cmake_minimum_required(VERSION 3.12)
project(koloshmet_lib)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Werror -Wall -Wextra)

add_subdirectory(util)
add_subdirectory(posix)
add_subdirectory(net)
add_subdirectory(async)
