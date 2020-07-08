#include "../lib/common.h"
#include "../lib/dict.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define HEADERS_BUCKET 10
#define SERVER_PORT 7777
#define MAX_LINE 4096
#define BACKLOG 128

int create_web_server_socket(int port);
int read_line(int fd, char *buf, int size);
int parse_header(int fd, dict *headers);
void add_request_line_to_dict(dict *headers, char *buf, size_t size);

int create_web_server_socket(int port) {
	int listen_fd, on = 1;
	struct sockaddr_in server_addr;

	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		error(1, errno, "create socket failed");
	}

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	if (bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		error(1, errno, "bind failed");
	}

	if (listen(listen_fd, BACKLOG) < 0) {
		error(1, errno, "listen failed");
	}

	return listen_fd;
}

int read_line(int fd, char *buf, int size) {
	char c = '\0';
	int i = 0;

	while (i < size - 1 && c != '\n') {
		if (recv(fd, &c, 1, 0) < 0) {
			c = '\n';
			continue;
		} else if (c == '\r') {
			if (recv(fd, &c, 1, MSG_PEEK) > 0 && c == '\n')
				recv(fd, &c, 1, 0);
			else
				c = '\n';
		}
		buf[i++] = c;
	}

	return i;
}

int parse_header(int fd, dict *headers) {
	int n = 0;
	char buf[MAX_LINE];

	if (read_line(fd, buf, MAX_LINE) <= 0) {
		printf("read request line failed");
		return 0;
	}

	add_request_line_to_dict(headers, buf, strlen(buf));

	while ((n = read_line(fd, buf, MAX_LINE)) > 0) {
		if (n == 1 && buf[0] == '\n') {
			break;
		}
		printf("header: %s", buf);
		bzero(&buf, MAX_LINE);
	}

	return n;
}

void add_request_line_to_dict(dict *headers, char *buf, size_t size) {
	int i = 0, n = 0;
	char values[2][256]; 
	char *keys[2] = {"method", "uri"};
	while (n < 2 && size--) {
		if (*buf == ' ') {
			values[n][i] = '\0';
			if (dictAdd(headers, keys[n], values[n]) == DICT_ERR) {
				printf("add header (%s) failed \n", keys[n]);
			}
			n++;
			i = 0;
			*buf++;
			continue;
		}
		values[n][i++] = *buf++;
	}

	buf[size-1] = '\0';
	if (dictAdd(headers, "version", buf) == DICT_ERR) {
		printf("add header (version) failed \n");
	}
}

int main(int argc, char **argv) {
	int listen_fd, conn_fd;
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	listen_fd = create_web_server_socket(SERVER_PORT);

	for (;;) {
		conn_fd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_len);
		if (conn_fd < 0 || errno == EINTR) {
			error(0, 0, "accept failed");
			continue;
		}

		printf("%s:%d connected \n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

		dict *headers;
		if ((headers = dictCreate(HEADERS_BUCKET)) == NULL) {
			error(0, 0, "create headers dict failed");
			continue;
		}

		parse_header(conn_fd, headers);

		printf("headers end\n");

		char *message = "HTTP/1.0 200 OK\r\nContent-Length: 2\r\n\r\nHi";
		write(conn_fd, message, strlen(message));
		close(conn_fd);
	}
}
