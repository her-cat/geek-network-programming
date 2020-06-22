add_executable(writev_client writev_client.c)
add_executable(writev_server writev_server.c)

target_link_libraries(writev_client)
target_link_libraries(writev_server)
