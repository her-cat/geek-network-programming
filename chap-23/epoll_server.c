#include "../lib/common.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define SERVER_PORT 8023
#define EPOLL_MAX_EVENTS 128

int create_tcp_server(int port) {
    int listen_fd, on = 1;
    struct sockaddr_in server_addr;

    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error(1, errno, "create socket failed");
    }

    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        error(1, errno, "bind failed");
    }

    if (listen(listen_fd, 128)) {
        error(1, errno, "listen failed");
    }

    return listen_fd;
}

int main(int argc, char *argv) {
    int listen_fd, epoll_fd, wakeup_num;
    struct epoll_event listen_event;
    struct epoll_event *events;

    listen_fd = create_tcp_server(SERVER_PORT);

    if ((epoll_fd = epoll_create1(0)) < 0)
        error(1, errno, "epoll create failed");

    listen_event.data.fd = listen_fd;
    listen_event.events = EPOLLIN | EPOLLET;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &listen_event) < 0)
        error(1, errno, "add listen fd to epoll failed");

    events = calloc(EPOLL_MAX_EVENTS, sizeof(listen_event));

    while (1) {
        wakeup_num = epoll_wait(epoll_fd, events, EPOLL_MAX_EVENTS, -1);
        printf("epoll_wait wakeup \n");

        for (int i = 0; i < wakeup_num; i++) {
            if ((events[i].events & EPOLLERR) || 
                (events[i].events & EPOLLHUP) || 
                !(events[i].events & EPOLLIN)) {
                printf("epoll error \n");
                close(events[i].data.fd);
                continue;
            }

            if (listen_fd == events[i].data.fd) {
                struct sockaddr_storage ss;

            }
        }
        
    }

    return EXIT_SUCCESS;
}