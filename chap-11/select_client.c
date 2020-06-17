#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

#include "../lib/common.h"

#define MAX_LINE 4096
#define SERVER_PORT 8011

int main(int argc, char **argv) {
	if (argc != 2) {
		error(1, 0, "usage: select_server <IP address>");
	}

	int socket_fd;
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0) {
		error(1, errno, "create socket failed");
	}

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

	int connect_rt = connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
	if (connect_rt < 0) {
		error(1, errno, "connect failed");
	}

	char send_line[MAX_LINE], recv_line[MAX_LINE + 1];
	int n;

	fd_set read_mask;
	fd_set all_reads;

	FD_ZERO(&all_reads);
	FD_SET(0, &all_reads);
	FD_SET(socket_fd, &all_reads);

	for (;;) {
		read_mask = all_reads;
		int rc = select(socket_fd + 1, &read_mask, NULL, NULL, NULL);
		if (rc <= 0) {
			error(1, errno, "select failed");
		}

		if (FD_ISSET(socket_fd, &read_mask)) {
			n = read(socket_fd, recv_line, MAX_LINE);
			if (n < 0) {
				error(1, errno, "read error");
			} else if (n == 0) {
				error(1, 0, "server terminated");
			}
			recv_line[n] = 0;
			fputs(recv_line, stdout);
			fputs("\n", stdout);
		}
		if (FD_ISSET(0, &read_mask)) {
			if (fgets(send_line, MAX_LINE, stdin) != NULL) {
				if (strncmp(send_line, "shutdown", 8) == 0) {
					FD_CLR(0, &all_reads);
					if (shutdown(socket_fd, SHUT_WR)) {
						error(1, errno, "shutdown failed");
					}
				} else if (strncmp(send_line, "close", 5) == 0) {
					FD_CLR(0, &all_reads);
					if (close(socket_fd)) {
						error(1, errno, "close failed");
					}
					sleep(6);
					exit(0);
				} else {
					int i = strlen(send_line);
					if (send_line[i - 1] == '\n') {
						send_line[i - 1] = 0;
					}

					printf("now sending: %s \n", send_line);
					size_t rt = write(socket_fd, send_line, strlen(send_line));
					if (rt < 0) {
						error(1, errno, "write failed");
					}
					printf("send bytes: %zu \n", rt);
				}
			}
		}
	}
}
