#include "../lib/common.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <arpa/inet.h>

#define SERVER_PORT 8023
#define MAX_LINE 4096

int connect_tcp_server(const char *address, int port) {
    int connect_fd, on = 1;
    struct sockaddr_in server_addr;

    if ((connect_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error(1, errno, "create socket failed");
    }

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, address, &server_addr.sin_addr);

    if (connect(connect_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        error(1, errno, "connect failed");
    }

    return connect_fd;
}

int main(int argc, char **argv) {
    int connect_fd, ready_num;
    char buf[MAX_LINE];
    struct pollfd fds[2];
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    if (argc != 2) {
        error(1, 1, "usage: select_trigger <IP address>");
    }

    connect_fd = connect_tcp_server(argv[1], SERVER_PORT);

    fds[0].fd = connect_fd;
    fds[0].events = POLLRDNORM;
    fds[1].fd = STDIN_FILENO;
    fds[1].events = POLLIN;

    while (1) {
        if ((ready_num = poll(fds, 2, -1)) < 0)
            error(1, errno, "poll error");

        if (fds[0].revents & (POLLRDNORM | POLLERR)) {
            printf("socket_fd can read.\n");
            sleep(1);
        }

        if (fds[1].revents & POLLIN) {
            if (fgets(buf, MAX_LINE, stdin) != NULL) {
                printf("keybord input: %s \n", buf);
            }
        }
    }

    return EXIT_SUCCESS;
}
