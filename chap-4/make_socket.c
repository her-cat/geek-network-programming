#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

int make_socket(uint16_t port) 
{
    int sock;
    struct sockaddr_in name;

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Create socket failed.");
        exit(EXIT_FAILURE);
    }

    name.sin_family = AF_INET; // IPv4
    name.sin_port = htons(port); // 指定端口
    name.sin_addr.s_addr = htonl(INADDR_ANY); // 通配地址

    // 将 IPv4 地址转换成通用地址格式，同时传递长度
    if (bind(sock, (struct sockaddr*) &name, sizeof(name)) < 0) {
        perror("Bind socket failed.");
        exit(EXIT_FAILURE);
    }

    return sock;
}

void main()
{
    int sock = make_socket(8081);

    if (listen(sock, 3) < 0) {
        perror("Listen socket failed.");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in cliaddr;
    socklen_t clilen;
    int cli_fd;

    while (1) {
        clilen = sizeof(cliaddr);
        cli_fd = accept(sock, (struct sockaddr *)&cliaddr, &clilen);

        printf("ip:%s port:%d \r\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

        char *msg = "hello, my bro.\r\n";
        send(cli_fd, msg, strlen(msg), 0);
        close(cli_fd);
    }

    close(sock);
}