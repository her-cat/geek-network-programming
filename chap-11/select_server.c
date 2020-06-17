#include "../lib/common.h"
#include <sys/socket.h>
#include <netinet/in.h>

#define SERVER_PORT 8011
#define SERVER_BACKLOG 128
#define MAX_LINE 4096

static int count;

static void sigint_handle(int signo) {
	printf("signal[%d] received %d datagram \n", signo, count);
	exit(0);
}

static void sigpipe_handle(int signo) {
	printf("signal[%d] received %d datagram \n", signo, count);
	exit(0);
}

int main(int argc, char **argv) {
	int listen_fd;
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0) {
		error(1, errno, "create socket failed");
	}

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(SERVER_PORT);

	int rt1 = bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
	if (rt1 < 0) {
		error(1, errno, "bind failed");
	}

	int rt2 = listen(listen_fd, SERVER_BACKLOG);
	if (rt2 < 0) {
		error(1, errno, "listen failed");
	}

	signal(SIGINT, sigint_handle);
	signal(SIGPIPE, sigpipe_handle);

	int conn_fd;
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	if ((conn_fd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_len)) < 0) {
		error(1, errno, "accept failed");
	}

	char message[MAX_LINE];
	count = 0;

	for (;;) {
		int n = read(conn_fd, message, MAX_LINE);
		if (n < 0) {
			error(1, 0, "read error");
		} else if (n == 0) {
			error(1, 0, "client closed");
		}

		count++;
		message[n] = 0;
		printf("received %d bytes: %s \n", n, message);

		char send_line[MAX_LINE];
		sprintf(send_line, "Hi, %s", message);

		sleep(5);

		int write_nc = send(conn_fd, send_line, strlen(send_line), 0);
		printf("send bytes: %d \n", write_nc);
		if (write_nc < 0) {
			error(1, errno, "write error");
		}
	}
}
