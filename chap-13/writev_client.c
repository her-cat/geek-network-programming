#include "../lib/common.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/uio.h>

#define SERVER_PORT 8013

int main(int argc, char **argv) {
	if (argc != 2) {
		error(1, 0, "usage: writev <IP address>");
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

	char buf[128];
	struct iovec iov[2];

	char *send_one = "hello, ";

	iov[0].iov_base = send_one;
	iov[0].iov_len = sizeof(send_one);
	iov[1].iov_base = buf;

	while (fgets(buf, sizeof(buf), stdin) != NULL) {
		iov[1].iov_len = strlen(buf);
		if (writev(socket_fd, iov, 2) < 0) {
			error(1, errno, "writev failure");
		}
	}

	exit(0);
}
