#include "../lib/common.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#define SERVER_PORT 8014
#define MAX_LINE 4096

static int count;

static void sigint_handle(int signo) {
	printf("received %d datagram\n", count);
	exit(0);
}

int main(int argc, char **argv) {
	int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_fd < 0) {
		error(1, errno, "create socket failed");
	}

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(SERVER_PORT);

	if (bind(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		error(1, errno, "bind failed");
	}

	signal(SIGINT, sigint_handle);

	count = 0;
	char message[MAX_LINE];

	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	int n = recvfrom(socket_fd, message, MAX_LINE, 0, (struct sockaddr *) &client_addr, &client_len);
	if (n < 0) {
		error(1, errno, "recvfrom failed");
	}

	message[n] = 0;
	printf("received %d bytes: %s \n", n, message);

	if (connect(socket_fd, (struct sockaddr *) &client_addr, client_len)) {
		error(1, errno, "connect failed");
	}


	while (strncmp(message, "goodbye", 7) != 0) {
		char send_line[MAX_LINE];
		sprintf(send_line, "Hi, %s", message);

		size_t rt = send(socket_fd, send_line, strlen(send_line), 0);
		if (rt < 0) {
			error(1, errno, "send failed");
		}

		printf("send bytes: %zu \n", rt);

		size_t rc = recv(socket_fd, message, MAX_LINE, 0);
		if (rc < 0) {
			error(1, errno, "recv failed");
		}

		count++;
	}

	return EXIT_SUCCESS;
}
