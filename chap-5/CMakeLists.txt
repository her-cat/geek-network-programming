add_executable(tcp_client tcp_client.c)
add_executable(tcp_server tcp_server.c)

target_link_libraries(tcp_client network)
target_link_libraries(tcp_server network)