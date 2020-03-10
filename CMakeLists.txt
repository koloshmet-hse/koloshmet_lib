cmake_minimum_required(VERSION 3.12)
project(koloshmet_lib)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Werror -Wall -Wextra)
if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_options(-g)
elseif (CMAKE_BUILD_TYPE MATCHES Release)
    add_compile_options(-O3)
endif()

add_subdirectory(util)
add_subdirectory(posix)
add_subdirectory(net)
add_subdirectory(async)
