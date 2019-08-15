cmake_minimum_required(VERSION 3.14)
project(koloshmet_lib)

set(CMAKE_CXX_STANDARD 17)

if (DEFINED APPLE)
    link_directories(/usr/local/opt/llvm/lib)
endif ()

add_library(koloshmet_lib util/exception.cpp)
target_include_directories(koloshmet_lib PUBLIC ${PROJECT_SOURCE_DIR})
