set(
    SRC
    stop_token/stop_token.cpp
)

add_library(k_async ${SRC})
target_link_libraries(k_async k_util)
add_dependencies(k_async k_util)
target_include_directories(k_async PUBLIC ${PROJECT_SOURCE_DIR})
