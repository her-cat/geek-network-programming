#include "../lib/common.h"
#include "./message_object.h"
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#define MAX_LINE 4096
#define SERVER_PORT 8012
#define KEEP_ALIVE_TIME 10
#define KEEP_ALIVE_INTERVAL 3
#define KEEP_ALIVE_PROBE_TIMES 3

int main(int argc, char **argv) {
	if (argc != 2) {
		error(1, 0, "usage: ping_client <IP address>");
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
	int connect_rt = connect(socket_fd, (struct sockaddr *) &server_addr,server_len);
	if (connect_rt < 0) {
		error(1, errno, "connect failed");
	}

	int n;
	char recv_line[MAX_LINE];

	fd_set read_mask;
	fd_set all_reads;

	struct timeval tv;
	int heartbeats = 0;

	tv.tv_sec = KEEP_ALIVE_TIME;
	tv.tv_usec = 0;

	messageObject message;

	FD_ZERO(&all_reads);
	FD_SET(socket_fd, &all_reads);

	for (;;) {
		read_mask = all_reads;
		int rc = select(socket_fd + 1, &read_mask, NULL, NULL, &tv);
		if (rc < 0) {
			error(1, errno, "select failed");
		}

		if (rc == 0) {
			if (++heartbeats > KEEP_ALIVE_PROBE_TIMES) {
				error(1, 0, "connection dead \n");
			}

			printf("sending heartbeat #%d \n", heartbeats);

			message.type = htonl(MSG_PING);
			rc = send(socket_fd, (char *) &message, sizeof(message), 0);
			if (rc < 0) {
				error(1, errno, "send failure");
			}

			tv.tv_sec = KEEP_ALIVE_INTERVAL;
			continue;
		}

		if (FD_ISSET(socket_fd, &read_mask)) {
			n = read(socket_fd, recv_line, MAX_LINE);
			if (n < 0) {
				error(1, errno, "read error");
			} else if (n == 0) {
				error(1, 0, "server terminated \n");
			}

			printf("received heartbeat, make heartbeats to 0 \n");

			heartbeats = 0;
			tv.tv_sec = KEEP_ALIVE_TIME;
		}
	}
}
