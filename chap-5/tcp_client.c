#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MESSAGE_SIZE 10240000

void send_data(int sockfd) {
    char *query = malloc(MESSAGE_SIZE + 1);

    for (int i = 0; i < MESSAGE_SIZE; i++) {
        query[i] = 'a';
    }
    query[MESSAGE_SIZE] = '\0';

    const char *cp = query;
    size_t remaining = strlen(query);
    while (remaining) {
        int written = send(sockfd, cp, remaining, 0);
        fprintf(stdout, "send into buffer %d \n", written);
        
        if (written <= 0) {
            perror("send failed");
            return;
        }

        remaining -= written;
        cp += written;
    }
}

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in servaddr;

    if (argc != 2) {
        perror("usage: client <IPaddress>");
        return EXIT_FAILURE;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(12345);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    int connect_rt = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (connect_rt < 0) {
        perror("connect failed");
    }

    send_data(sockfd);
    exit(0);
}
