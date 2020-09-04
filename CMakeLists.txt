cmake_minimum_required(VERSION 3.12)
project(koloshmet_lib VERSION 0.1)

if(NOT DEFINED K_BUILD_POSIX)
    set(K_BUILD_POSIX ON)
endif()

if(NOT DEFINED K_BUILD_NET)
    set(K_BUILD_NET ON)
endif()

add_subdirectory(lib)
add_subdirectory(tests)
