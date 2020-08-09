#include "../lib/common.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define SERVER_PORT 8023
#define EPOLL_MAX_EVENTS 128
#define MAX_LINE 4096

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

char rot13_char(char c) {
    if ((c >= 'a' && c <= 'm') || (c >= 'A' || c <= 'M'))
        return c + 13;
    else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
        return c - 13;
    else 
        return c;
}

int main(int argc, char *argv) {
    int listen_fd, epoll_fd, conn_fd, wakeup_num, received_num;
    struct epoll_event event;
    struct epoll_event *events;
    char buf[MAX_LINE];

    listen_fd = create_tcp_server(SERVER_PORT);

    if ((epoll_fd = epoll_create1(0)) < 0)
        error(1, errno, "epoll create failed");

    event.data.fd = listen_fd;
    event.events = EPOLLIN | EPOLLET;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &event) < 0)
        error(1, errno, "add listen fd to epoll failed");

    events = calloc(EPOLL_MAX_EVENTS, sizeof(event));

    while (1) {
        wakeup_num = epoll_wait(epoll_fd, events, EPOLL_MAX_EVENTS, -1);

        for (int i = 0; i < wakeup_num; i++) {
            if ((events[i].events & EPOLLERR) || 
                (events[i].events & EPOLLHUP) || 
                !(events[i].events & EPOLLIN)) {
                printf("epoll error \n");
                close(events[i].data.fd);
                continue;
            }

            if (listen_fd == events[i].data.fd) {
                printf("client connected!\n");
                struct sockaddr_storage client_addr;
                socklen_t client_len = sizeof(client_addr);

                if ((conn_fd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_len)) < 0)
                    error(0, errno, "accept error");

                fcntl(conn_fd, F_SETFL, O_NONBLOCK);
                event.data.fd = conn_fd;
                event.events = EPOLLIN; /* level-triggered */

                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &event) < 0)
                    error(0, errno, "add client fd failed");
                continue;
            }

            bzero(&buf, MAX_LINE);
            received_num = read(events[i].data.fd, buf, MAX_LINE);
            if (received_num < 0) {
                error(0, errno, "read error");
                continue;
            } else if (received_num == 0) {
                close(events[i].data.fd);
                event.data.fd = events[i].data.fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, &event);
                printf("client closed \n");
                continue;
            }

            printf("recevied %d bytes: %s", received_num, buf);

            for (int j = 0; j < received_num; j++) {
                buf[j] = rot13_char(buf[j]);
            }

            if (write(events[i].data.fd, buf, received_num) < 0)
                error(0, errno, "write error");
        }
    }

    close(listen_fd);
    close(epoll_fd);

    return EXIT_SUCCESS;
}