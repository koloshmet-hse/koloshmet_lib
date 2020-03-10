set(
    SRC
    coding/url.cpp
    http/message.cpp
)

add_library(k_net ${SRC})
target_link_libraries(k_net k_util)
add_dependencies(k_net k_util)
target_include_directories(k_net PUBLIC ${PROJECT_SOURCE_DIR})
