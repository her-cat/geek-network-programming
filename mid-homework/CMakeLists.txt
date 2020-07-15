add_executable(telnet_server telnet_server.c)
add_executable(telnet_client telnet_client.c)

target_link_libraries(telnet_server)
target_link_libraries(telnet_client)
