#include "../lib/common.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>


void read_data(int sockfd) {
    ssize_t n;
    char buf[1024];

    int time = 0;
    for (;;) {
        fprintf(stdout, "block in read \n");
        if ((n = readn(sockfd, buf, 1024)) == 0) {
            fprintf(stdout, "client closed \n");
            return;
        }

        time++;
        fprintf(stdout, "1K read for %d \n", time);
        usleep(1000);
    }
}

int main(int argc, char **argv) {
    int listenfd, connfd;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(12345);

    // bind 到本地地址，端口为 12345
    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    // listen 的 backlog 为 1024
    listen(listenfd, 1024);

    for (;;) {
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
        read_data(connfd);
        close(connfd);
    }
}
