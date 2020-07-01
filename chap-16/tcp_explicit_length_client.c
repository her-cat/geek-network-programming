#include "../lib/common.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_PORT 8016

int main(int argc, char **argv) {
	if (argc != 2) {
		error(1, 0, "usage: tcp_explicit_length_client <IP address>");
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

	struct {
		u_int32_t length;
		u_int32_t type;
		char buf[128];
	} message;

	while (fgets(message.buf, sizeof(message.buf), stdin) != NULL) {
		int n = strlen(message.buf);
		message.length = htonl(n);
		message.type = 1;
		printf("sending bytes: %s", message.buf);

		if (send(socket_fd, (char *) &message, sizeof(message.length) + sizeof(message.type) + n, 0) < 0) {
			error(1, errno, "send failed");
		}
	}

	return EXIT_SUCCESS;
}
