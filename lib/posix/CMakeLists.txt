set(
    SRC
    file_descriptor/fd.cpp
    file_descriptor/unique_fd.cpp
    file_descriptor/shared_fd.cpp
    file_descriptor/syscalls.cpp
    file_descriptor/fd_stream.cpp
    subprocess/environment_variable.cpp
    subprocess/subprocess.cpp
    net/socket.cpp
    net/socket_pool.cpp
    net/server.cpp
    net/client.cpp
)

add_library(k_posix ${SRC})

target_link_libraries(k_posix k_async)
add_dependencies(k_posix k_async)
target_include_directories(k_posix PUBLIC ${CMAKE_SOURCE_DIR}/lib)
