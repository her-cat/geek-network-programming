add_executable(select_trigger select_trigger.c)
add_executable(poll_trigger poll_trigger.c)
add_executable(epoll_server epoll_server.c)

target_link_libraries(select_trigger)
target_link_libraries(poll_trigger)
target_link_libraries(epoll_server)
