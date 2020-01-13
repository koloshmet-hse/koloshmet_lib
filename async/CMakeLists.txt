cmake_minimum_required(VERSION 3.12)

set(
    SRC
    stop_token/stop_token.cpp
)

add_library(lib_async ${SRC})
target_link_libraries(lib_async lib_util)
add_dependencies(lib_async lib_util)
target_include_directories(lib_async PUBLIC ${PROJECT_SOURCE_DIR})