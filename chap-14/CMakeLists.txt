add_executable(udp_connect_client udp_connect_client.c)
add_executable(udp_connect_server udp_connect_server.c)

target_link_libraries(udp_connect_client)
target_link_libraries(udp_connect_server)
