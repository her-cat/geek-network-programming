#include "../lib/common.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define SERVER_PORT 8026
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

void connection_loop(int fd) {
    int result;
    pthread_t tid;
    char buf[MAX_LINE];

    tid = pthread_self();

    while (1) {
        bzero(&buf, MAX_LINE);
        result = read(fd, buf, MAX_LINE);
        if (result < 0) {
            error(1, errno, "read error");
        } else if (result == 0) {
            printf("client closed \n");
            close(fd);
            break;
        }

        printf("[tid:%ld] received %d bytes: %s", tid, result, buf);

        for (int i = 0; i < result; i++) {
            buf[i] = rot13_char(buf[i]);
        }
        
        if (write(fd, buf, result) < 0)
            error(1, errno, "write error");
    }
}

void *thread_run(void *arg) {
    int *fd = (int *) arg;
    /* 分离当前线程，在线程终止后能够自动回收相关的线程资源 */
    pthread_detach(pthread_self());
    connection_loop(*fd);
}

int main(int argc, char *argv) {
    pthread_t tid;
    int listen_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    listen_fd = create_tcp_server(SERVER_PORT);

    while (1) {
        int conn_fd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_len);
        if (conn_fd < 0) {
            error(0, errno, "accept error");
            continue;
        }
        printf("client connected! \n");
        /* 为每一个连接创建一个线程进行通信 */
        pthread_create(&tid, NULL, thread_run, &conn_fd);
    }

    return EXIT_SUCCESS;
}
