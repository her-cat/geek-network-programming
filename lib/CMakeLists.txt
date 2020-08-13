add_library(dict dict.c)
add_library(acceptor acceptor.c)

target_link_libraries(dict)
target_link_libraries(acceptor)
