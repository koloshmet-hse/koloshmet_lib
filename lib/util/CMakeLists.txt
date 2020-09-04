set(
    SRC
    type/utils.cpp
    type/type_list.cpp
    type/list_utils.cpp
    exception/exception.cpp
    memory/reserve_t.cpp
    memory/move_on_rvalue_ptr.cpp
    string/utils.cpp
    tree_value/tree_value.cpp
    tree_value/json_io.cpp
    io/binary_io.cpp
    container/slice.cpp
    container/utils.cpp
    opt/opt.cpp
    opt/command.cpp
    opt/options.cpp
)

add_library(k_util ${SRC})
if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0.0)
    if (CMAKE_CXX_COMPILER_ID MATCHES Clang)
        target_link_libraries(k_util c++fs)
    elseif (CMAKE_CXX_COMPILER_ID MATCHES GNU)
        target_link_libraries(k_util stdc++fs)
    endif()
endif()

target_include_directories(k_util PUBLIC ${CMAKE_SOURCE_DIR}/lib)
