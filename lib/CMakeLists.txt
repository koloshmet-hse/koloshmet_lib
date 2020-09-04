set(CMAKE_CXX_STANDARD 20)

add_compile_options(-Werror -Wall -Wextra)

add_subdirectory(util)

if(K_BUILD_POSIX)
    set(K_BUILD_ASYNC ON)
endif()

if(K_BUILD_ASYNC)
    add_subdirectory(async)
    if(K_BUILD_POSIX)
        add_subdirectory(posix)
    endif()
endif()

if(K_BUILD_NET)
    add_subdirectory(net)
endif()
