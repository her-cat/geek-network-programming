#include "../lib/common.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#define SERVER_PORT 8016
#define BACKLOG 128

static int count = 0;

static void sigint_handle(int signo) {
	printf("\nreceived count: %d \n", count);
	exit(0);
}

size_t read_message(int fd, char *buffer, size_t length) {
	u_int32_t msg_length;
	u_int32_t msg_type;
	int rc;

	rc = readn(fd, &msg_length, sizeof(msg_length));
	if (rc != sizeof(u_int32_t)) {
		return rc < 0 ? -1 : 0;
	}

	msg_length = ntohl(msg_length);

	rc = readn(fd, &msg_type, sizeof(msg_type));
	if (rc != sizeof(u_int32_t)) {
		return rc < 0 ? -1 : 0;
	}

	if (msg_length > length) {
		return -1;
	}

	rc = readn(fd, buffer, msg_length);
	if (rc != msg_length) {
		return rc < 0 ? -1 : 0;
	}

	return rc;
}

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

	int on = 1;
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	if (bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		error(1, errno, "bind failed");
	}

	if (listen(listen_fd, BACKLOG) < 0) {
		error(1, errno, "listen failed");
	}

	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, sigint_handle);

	int conn_fd;
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	if ((conn_fd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_len)) < 0) {
		error(1, errno, "accept failed");
	}

	char buf[128];
	while (1) {
		int n = read_message(conn_fd, buf, sizeof(buf));
		if (n < 0) {
			error(1, errno, "error read message");
		} else if (n == 0) {
			error(1, 0, "client closed");
		}

		count++;
		printf("received %d bytes: %s", n, buf);
	}
}
