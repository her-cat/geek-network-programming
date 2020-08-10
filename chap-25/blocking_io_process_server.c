#include "../lib/common.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>

#define SERVER_PORT 8025
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
    if ((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M'))
        return c + 13;
    else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
        return c - 13;
    else 
        return c;
}

void child_run(int fd) {
    char buf[MAX_LINE];
    int result;

    while (1) {
        bzero(&buf, MAX_LINE);
        result = read(fd, buf, MAX_LINE);
        if (result == 0) {
            printf("pid: %d client closed \n", getpid());
            break;
        } else if (result < 0) {
            error(1, errno, "read error");
            break;
        }

        printf("received %d bytes: %s", result, buf);

        for (int i = 0; i < result; i++) {
            buf[i] = rot13_char(buf[i]);
        }

        if (write(fd, buf, result) < 0)
            error(1, errno, "write error");
    }
}

void sigchld_handler(int signo) {
    while (waitpid(-1, 0, WNOHANG) > 0);
    printf("received SIGCHLD sinal \n");
}

int main(int argc, int argv) {
    int listen_fd, conn_fd;

    listen_fd = create_tcp_server(SERVER_PORT);

    signal(SIGCHLD, sigchld_handler);

    while (1) {
        struct sockaddr_storage client_addr;
        socklen_t client_len = sizeof(client_addr);

        conn_fd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_len);
        if (conn_fd < 0) {
            error(0, errno, "accept error");
            continue;
        }

        printf("client connected \n");

         /* fork 时，会将监听套接字、链接套接字的引用计数+1 */
        if (fork() == 0) {
            close(listen_fd); /* 子进程不关心监听套接字，所以将其关闭，监听套接字引用计数-1 */
            child_run(conn_fd);
            exit(0);
        } else {
            close(conn_fd); /* 父进程不关心连接套接字，所以将其关闭，连接套接字引用计数-1 */
        }
    }

    return EXIT_SUCCESS;
}
