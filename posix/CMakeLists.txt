cmake_minimum_required(VERSION 3.12)

set(
    SRC
    file_descriptor/fd.cpp
    file_descriptor/unique_fd.cpp
    file_descriptor/fd_stream.cpp
    subprocess/subprocess.cpp
    net/socket.cpp
        net/socket_pool.cpp
    net/server.cpp
)

add_library(lib_posix ${SRC})
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_link_libraries(lib_posix c++fs)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_link_libraries(lib_posix stdc++fs)
endif()

target_link_libraries(lib_posix lib_util)
add_dependencies(lib_posix lib_util)
target_include_directories(lib_posix PUBLIC ${PROJECT_SOURCE_DIR})
