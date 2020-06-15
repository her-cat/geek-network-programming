#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_LINE 1024

int main(int argc, char **argv) {
	if (argc != 2) {
		perror("usage: unix_client <local_path>");
		return EXIT_FAILURE;
	}

	int sock_fd;
	struct sockaddr_un server_addr;

	sock_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
	if (sock_fd < 0) {
		perror("create socket failed");
		return EXIT_FAILURE;
	}

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sun_family = AF_LOCAL;
	strcpy(server_addr.sun_path, argv[1]);

	if (connect(sock_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		perror("connect failed");
		return EXIT_FAILURE;
	}

	char send_line[MAX_LINE];
	bzero(send_line, MAX_LINE);
	char recv_line[MAX_LINE];

	while (fgets(send_line, MAX_LINE, stdin) != NULL) {
		int bytes = sizeof(send_line);
		if (write(sock_fd, send_line, bytes) != bytes) {
			perror("write error");
			return EXIT_FAILURE;
		}

		if (read(sock_fd, recv_line, MAX_LINE) == 0) {
			perror("server terminated prematurely");
			return EXIT_FAILURE;
		}

		fputs(recv_line, stdout);
	}

	close(sock_fd);

	return EXIT_SUCCESS;
}