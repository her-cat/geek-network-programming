#include "../lib/common.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_PORT 8014
#define MAX_LINE 4096

int main(int argc, char **argv) {
	if (argc != 2) {
		error(1, 0, "usage: udp_connect_client <IP address>");
	}

	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0) {
		error(1, errno, "create socket failed");
	}

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

	socklen_t server_len = sizeof(server_addr);

	if (connect(socket_fd, (struct sockaddr *) &server_addr, server_len) < 0) {
		error(1, errno, "connect failed");
	}

	int n;
	char send_line[MAX_LINE], recv_line[MAX_LINE + 1];

	while (fgets(send_line, MAX_LINE, stdin) != NULL) {
		int i = strlen(send_line);
		if (send_line[i - 1] == '\n') {
			send_line[i - 1] = 0;
		}

		printf("now sending %s \n", send_line);
		size_t rt = send(socket_fd, send_line, i, 0);
		if (rt < 0) {
			error(1, errno, "send failed");
		}

		printf("send bytes: %zu \n", rt);

		recv_line[0] = 0;
		n = recv(socket_fd, recv_line, MAX_LINE, 0);
		if (n < 0) {
			error(1, errno, "recv failed");
		}

		recv_line[n] = 0;
		fputs(recv_line, stdout);
		fputs("\n", stdout);
	}

	return EXIT_SUCCESS;
}
