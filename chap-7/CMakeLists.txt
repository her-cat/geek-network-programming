add_executable(unix_client unix_client.c)
add_executable(unix_server unix_server.c)

target_link_libraries(unix_client)
target_link_libraries(unix_server)
