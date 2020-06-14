#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/un.h>
#include <zconf.h>
#include <string.h>
#include <errno.h>

#define BACKLOG 128
#define BUFFER_SIZE 1024

#define MAX_LINE BUFFER_SIZE

int main(int argc, char **argv) {
	if (argc != 2) {
		perror("usage: unix_server <local_path> \n");
		return EXIT_FAILURE;
	}

	int listen_fd, conn_fd;
	socklen_t client_len;
	struct sockaddr_un client_addr, server_addr;

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0) {
		perror("socket create failed \n");
		return EXIT_FAILURE;
	}

	char *local_path = argv[1];
	// 删除已存在的文件，保持幂等性
	unlink(local_path);
	bzero(&server_addr, sizeof(server_addr));
	// AF_LOCAL = AF_UNIX
	server_addr.sun_family = AF_LOCAL;
	strcpy(server_addr.sun_path, local_path);

	if (bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		perror("bind failed \n");
		return EXIT_FAILURE;
	}

	if (listen(listen_fd, BACKLOG) < 0) {
		perror("listen failed \n");
		return EXIT_FAILURE;
	}

	client_len = sizeof(client_addr);
	if ((conn_fd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_len)) < 0) {
		if (errno == EINTR) {
			perror("accept failed \n"); // back to for()
		} else {
			perror("accept failed \n");
		}

		return EXIT_FAILURE;
	}

	char buf[BUFFER_SIZE];

	while (1) {
		bzero(buf, sizeof(buf));

		if (read(conn_fd, buf, BUFFER_SIZE) == 0) {
			printf("client quit \n");
			break;
		}

		printf("received: %s \n", buf);

		char send_line[MAX_LINE];
		sprintf(send_line, "Hi, %s", buf);

		int bytes = sizeof(send_line);

		if (write(conn_fd, send_line, bytes) != bytes) {
			perror("write error");
			return EXIT_FAILURE;
		}
	}

	close(listen_fd);
	close(conn_fd);

	return EXIT_SUCCESS;
}