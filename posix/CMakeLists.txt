cmake_minimum_required(VERSION 3.12)

set(
    SRC
    file_descriptor/fd.cpp
    file_descriptor/unique_fd.cpp
    file_descriptor/syscalls.cpp
    file_descriptor/fd_stream.cpp
    subprocess/subprocess.cpp
    net/socket.cpp
    net/socket_pool.cpp
    net/server.cpp
    net/client.cpp
)

add_library(lib_posix ${SRC})

target_link_libraries(lib_posix lib_async)
add_dependencies(lib_posix lib_async)
target_include_directories(lib_posix PUBLIC ${PROJECT_SOURCE_DIR})
