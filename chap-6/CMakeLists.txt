add_executable(udp_client udp_client.c)
add_executable(udp_server udp_server.c)

target_link_libraries(udp_client network)
target_link_libraries(udp_server network)