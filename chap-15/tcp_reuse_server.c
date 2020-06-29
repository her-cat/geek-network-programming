#include "../lib/common.h"
#include <sys/socket.h>
#include <netinet/in.h>

#define SERVER_PORT 8015
#define BACKLOG 128
#define MAX_LINE 4096

int main(int argc, char **argv) {
	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0) {
		error(1, errno, "create socket failed");
	}

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		error(1, errno, "bind failed");
	}

	if (listen(listen_fd, BACKLOG) < 0) {
		error(1, errno, "listen failed");
	}

	signal(SIGPIPE, SIG_IGN);

	int conn_fd;
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	if ((conn_fd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_len)) < 0) {
		error(1, errno, "accept failed");
	}

	char message[MAX_LINE];

	for (;;) {
		int n = read(conn_fd, message, MAX_LINE);
		if (n < 0) {
			error(1, errno, "read error");
		} else if (n == 0) {
			error(1, 0, "client closed");
		}

		message[n] = 0;
		printf("received %d bytes: %s", n, message);
	}
}