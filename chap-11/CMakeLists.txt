add_executable(select_client select_client.c)
add_executable(select_server select_server.c)

target_link_libraries(select_client)
target_link_libraries(select_server)
