cmake_minimum_required(VERSION 3.14)

add_library(lib_util exception/exception.cpp)
target_include_directories(lib_util PUBLIC ${PROJECT_SOURCE_DIR})