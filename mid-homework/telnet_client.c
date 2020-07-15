#include "../lib/common.h"
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_PORT 8017
#define MAX_LINE 4096

int connect_server(const char *ip_addr, const int port);
int read_message(int socket_fd, char *message, size_t size);
int send_message(int socket_fd, char *message);

int connect_server(const char *ip_addr, const int port) {
    int socket_fd;

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error(1, errno, "create socket failed");
    }

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip_addr, &server_addr.sin_addr);

    if (connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        error(1, errno, "connect failed");
    }

    return socket_fd;
}

int read_message(int socket_fd, char *message, size_t size) {
    int recv_rt = read(socket_fd, message, size);
	if (recv_rt < 0)
		printf("read failed \n");
	else if (recv_rt == 0)
		printf("server terminated \n");

	return recv_rt;
}

int send_message(int socket_fd, char *message) {
	int send_rt;

	send_rt = send(socket_fd, message, strlen(message), 0);
	if (send_rt < 0)
		printf("send failed \n");
	else if (send_rt == 0)
		printf("server terminated \n");

	return send_rt;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        error(1, 0, "usage: telnet_client <IP address> \n");
    }

    int socket_fd;
    char send_line[MAX_LINE], recv_line[MAX_LINE];
    fd_set read_fds, read_mask;
    struct timeval timeout;

    socket_fd = connect_server(argv[1], SERVER_PORT);

    FD_ZERO(&read_fds);
    FD_SET(0, &read_fds);
    FD_SET(socket_fd, &read_fds);

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    while (1) {
        read_mask = read_fds;
        bzero(&send_line, strlen(send_line));
        bzero(&recv_line, strlen(recv_line));

        if (select(socket_fd + 1, &read_mask, NULL, NULL, &timeout) < 0) {
            error(1, errno, "select failed");
        }

        if (FD_ISSET(socket_fd, &read_mask)) {
            if (read_message(socket_fd, recv_line, MAX_LINE) == 0)
                break;
            fputs(recv_line, stdout);
            fputs("\n", stdout);
        }

        if (FD_ISSET(0, &read_mask)) {
            if(fgets(send_line, MAX_LINE, stdin) == NULL)
                continue;

            if (send_message(socket_fd, send_line) == 0)
                break;
        }
    }

    return EXIT_SUCCESS;
}
