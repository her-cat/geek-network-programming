#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_PORT 8090
#define MAX_LINE 4096

int main(int argc, char **argv) {
	if (argc != 2) {
		perror("usage: udp_client <IP address>");
		return EXIT_FAILURE;
	}

	int socket_fd;
	// 创建一个 UDP 套接字
	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

	socklen_t server_len = sizeof(server_addr);

	struct sockaddr_in *reply_addr = malloc(server_len);

	char send_line[MAX_LINE], recv_line[MAX_LINE + 1];
	socklen_t len;
	int n;

	while (fgets(send_line, MAX_LINE, stdin) != NULL) {
		int i = strlen(send_line);
		if (send_line[i - 1] == '\n') {
			send_line[i - 1] = 0;
		}

		printf("now sending %s \n", send_line);
		size_t rt = sendto(socket_fd, send_line, strlen(send_line), 0, (struct sockaddr *) &server_addr, server_len);
		if (rt < 0) {
			perror("send failed");
			return EXIT_FAILURE;
		}

		printf("send bytes: %zu \n", rt);

		len = 0;
		n = recvfrom(socket_fd, recv_line, MAX_LINE, 0, (struct sockaddr *)reply_addr, &len);
		if (n < 0) {
			perror("recvfrom failed");
			return EXIT_FAILURE;
		}

		recv_line[n] = 0;
		fputs(recv_line, stdout);
		fputs("\n", stdout);
	}
}
