#include "./message_object.h"
#include "../lib/common.h"
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SERVER_PORT 8012
#define BACKLOG 128

int main(int argc, char **argv) {
	if (argc != 2) {
		error(1, 0, "usage: ping_server <sleeping_time>");
	}

	int sleeping_time = atoi(argv[1]);

	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0) {
		error(0, errno, "create socket failed");
	}

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(SERVER_PORT);

	if (bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		error(1, errno, "bind failed");
	}

	if (listen(listen_fd, BACKLOG) < 0) {
		error(1, errno, "listen failed");
	}

	int conn_fd;
	struct sockaddr_in client_addr;
	socklen_t  client_len = sizeof(client_addr);

	if ((conn_fd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_len)) < 0) {
		error(1, errno, "accept failed");
	}

	messageObject message;

	for (;;) {
		int n = read(conn_fd, (char *) &message, sizeof(message));
		if (n < 0) {
			error(1, errno, "read error");
		} else if (n == 0) {
			error(1, 0, "client closed");
		}

		printf("received: %d bytes \n", n);

		switch (ntohl(message.type)) {
			case MSG_TYPE1:
				printf("process MSG_TYPE1 \n");
				break;
			case MSG_TYPE2:
				printf("process MSG_TYPE2 \n");
				break;
			case MSG_PING: {
				messageObject pong_message;
				pong_message.type = MSG_PONG;

				sleep(sleeping_time);

				if (send(conn_fd, (char *) &pong_message, sizeof(pong_message), 0) < 0) {
					error(1, errno, "send failure");
				}

				break;
			}
			default:
				error(1, 0, "unknown message type (%d) \n", ntohl(message.type));
		}
	}
}
