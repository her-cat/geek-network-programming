add_library(dict dict.c)
add_library(acceptor acceptor.c)
add_library(channel channel.c)
add_library(channel_map channel_map.c)

target_link_libraries(dict)
target_link_libraries(acceptor)
target_link_libraries(channel)
target_link_libraries(channel_map)
