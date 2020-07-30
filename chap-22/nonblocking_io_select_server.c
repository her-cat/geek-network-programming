#include "../lib/common.h"
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <fcntl.h>

#define SERVER_PORT 8022
#define FD_MAX_SIZE 128
#define MAX_LINE 4096

struct buffer {
    int fd; /* 连接字 */
    char buf[MAX_LINE]; /* 实际缓冲 */
    size_t read_idx; /* 缓冲读取位置 */
    size_t write_idx; /* 缓冲写入位置 */
    int readable; /* 是否可读 */
};

char rot13_char(char c) {
    if ((c >= 'a' && c <= 'm') || (c >= 'A' || c <= 'M'))
        return c + 13;
    else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
        return c - 13;
    else 
        return c;
}

struct buffer *alloc_buffer() {
    struct buffer *b = malloc(sizeof(struct buffer));
    if (!b)
        return NULL;

    b->fd = 0;
    b->read_idx = 0;
    b->write_idx = 0;
    b->readable = 0;

    return b;
}

void free_buffer(struct buffer *b) {
    free(b);
}

int on_socket_readable(int fd, struct buffer *b) {
    int i;
    ssize_t result;
    char buf[MAX_LINE];

    while (1) {
        result = recv(fd, buf, sizeof(buf), 0);
        if (result <= 0)
            break;

        for (i = 0; i < result; i++) {
            if (b->write_idx < sizeof(b->buf))
                b->buf[b->write_idx++] = rot13_char(buf[i]);
            if (buf[i] == '\n')
                b->readable = 1; /* 缓冲区可读 */
        }
    }

    if (result == 0)
        return 1;
    else if (result < 0)
        return errno == EAGAIN ? 0 : -1;
    
    return 0;
}

int on_socket_writable(int fd, struct buffer *b) {
    ssize_t result;
    while (b->read_idx < b->write_idx) {
        result = send(fd, b->buf + b->read_idx,  b->write_idx - b->read_idx, 0);
        if (result < 0)
            return errno == EAGAIN ? 0 : -1;
        b->read_idx += result;
    }

    if (b->read_idx == b->write_idx)
        b->read_idx = b->write_idx = 0;
    
    b->readable = 0;

    return 0;
}

int create_nonblocking_tcp_server(int port) {
    int listen_fd, on = 1;
    struct sockaddr_in server_addr;

    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error(1, errno, "create socket failed");

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    /* 地址重用 */
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    /* 设置为非阻塞 fd */
    fcntl(listen_fd, F_SETFL, O_NONBLOCK);

    if (bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
        error(1, errno, "bind failed");
    
    if (listen(listen_fd, 128) < 0)
        error(1, errno, "listen failed");
    
    return listen_fd;
}

int main(int argc, char *argv) {
    int listen_fd, conn_fd, max_fd;
    struct buffer *buf[FD_MAX_SIZE];
    fd_set read_fds, write_fds, exp_fds;

    listen_fd = create_nonblocking_tcp_server(SERVER_PORT);

    for (int i = 0; i < FD_MAX_SIZE; i++) {
        buf[i] = alloc_buffer();
    }

    while (1) {
        max_fd = listen_fd;
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        FD_ZERO(&exp_fds);

        FD_SET(listen_fd, &read_fds);

        for (int i = 0; i < FD_MAX_SIZE; i++) {
            if (buf[i]->fd <= 0)
                continue;
            if (buf[i]->fd > max_fd)
                max_fd = buf[i]->fd;
            FD_SET(buf[i]->fd, &read_fds);
            if (buf[i]->readable)
                FD_SET(buf[i]->fd, &write_fds);
        }
        
        if (select(max_fd + 1, &read_fds, &write_fds, &exp_fds, NULL) < 0)
            error(1, errno, "select error");
        
        if (FD_ISSET(listen_fd, &read_fds)) {
            printf("client connected \n");

            struct sockaddr_storage client_addr;
            socklen_t client_len = sizeof(client_addr);

            conn_fd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_len);
            if (conn_fd < 0) {
                error(1, errno, "accept failed");
            } else if (conn_fd > FD_MAX_SIZE) {
                printf("too many connections \n");
                close(conn_fd);
            } else {
                fcntl(conn_fd, F_SETFL, O_NONBLOCK);
                if (buf[conn_fd]->fd == 0)
                    buf[conn_fd]->fd = conn_fd;
                else
                    printf("too many connections \n");
            }
        }

        for (int i = 0; i < max_fd + 1; i++) {
            int err = 0;
            if (i == listen_fd)
                continue;
            if (FD_ISSET(i, &read_fds))
                err = on_socket_readable(i, buf[i]);
            if (err == 0 && FD_ISSET(i, &write_fds))
                err = on_socket_writable(i, buf[i]);
            if (err) {
                buf[i]->fd = 0;
                close(i);
            }
        }
    }
    

    return EXIT_SUCCESS;
}
