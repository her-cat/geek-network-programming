#define __USE_XOPEN 1

#include "../lib/common.h"
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>

#define SERVER_PORT 8021
#define POLL_FD_SIZE 128

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
    int listen_fd, conn_fd, fd_idx = 0, ready_num;
    struct pollfd fds[POLL_FD_SIZE];
    struct sockaddr_in client_addr;

    listen_fd = create_tcp_server(SERVER_PORT);

    fds[fd_idx].fd = listen_fd;
    fds[fd_idx].events = POLLRDNORM;

    for (int i = 1; i < POLL_FD_SIZE; i++) {
        fds[i].fd = -1;
    }

    for (;;) {
        if ((ready_num = poll(fds, POLL_FD_SIZE, -1)) < 0) {
            error(1, errno, "poll failed");
        }

        if (fds[0].revents & POLLRDNORM) {
            socklen_t client_len = sizeof(client_addr);
            conn_fd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_len);

            for (int i = 1; i < POLL_FD_SIZE; i++) {
                if (fds[i].fd < 0) {
                    fds[i].fd = conn_fd;
                    fds[i].events = POLLRDNORM;
                    break;
                }
            }

            if (fd_idx == POLL_FD_SIZE) {
                error(1, errno, "can not hold so many clients");
            }

            if (--ready_num <= 0)
                continue;
        }

    }

    return EXIT_SUCCESS;
}