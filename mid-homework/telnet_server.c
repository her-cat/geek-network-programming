#include "../lib/common.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#define SERVER_PORT 8017
#define BACKLOG 128
#define MAX_LINE 4096
#define FILE_MAX_BUFFER 256

int create_tcp_server_socket(int port);
int accept_client_connect(int listen_fd, struct sockaddr_in *client_addr, socklen_t *client_len);
void handle_client_request(int conn_fd, struct sockaddr_in client_addr);
int read_client_message(int conn_fd, char *message, size_t size);
int reply_client_message(int conn_fd, char *message);
char *execute_cmd(char *cmd);

int create_tcp_server_socket(int port) {
	int listen_fd, on = 1;

	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		error(1, errno, "create socket failed");
	}

	struct sockaddr_in server_addr;
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

int accept_client_connect(int listen_fd, struct sockaddr_in *client_addr, socklen_t *client_len) {
	int conn_fd = accept(listen_fd, (struct sockaddr *) client_addr, client_len);
	if (conn_fd < 0 || errno == EINTR) {
		printf("connect failed \n");
		return -1;
	}

	return conn_fd;
}

void handle_client_request(int conn_fd, struct sockaddr_in client_addr) {
	int recv_rt;
	char recv_line[MAX_LINE];

	printf("%s:%d connected... \n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

	for (;;) {
		recv_rt = read_client_message(conn_fd, recv_line, MAX_LINE);
		if (recv_rt < 0)
			continue;
		else if (recv_rt == 0)
			break;

		if (strncmp(recv_line, "quit", 4) == 0) {
			reply_client_message(conn_fd, "good bye! \n");
			printf("client quit \n");
			break;
		} else if (strncmp(recv_line, "ls", 2) == 0) {
			char *result = execute_cmd("ls");
			if (reply_client_message(conn_fd, result) == 0)
				break;
		} else if (strncmp(recv_line, "pwd", 3) == 0) {
			char *result = getcwd(NULL, 0);
			if (reply_client_message(conn_fd, result) == 0)
				break;
		} else {
			if (reply_client_message(conn_fd, recv_line) == 0)
				break;
		}
	}
}

int read_client_message(int conn_fd, char *message, size_t size) {
	int recv_rt = read(conn_fd, message, size);
	if (recv_rt < 0)
		printf("read failed \n");
	else if (recv_rt == 0)
		printf("client closed \n");

	message[recv_rt] = '\0';
	printf("received %d bytes:%s", recv_rt - 2, message);

	return recv_rt;
}

int reply_client_message(int conn_fd, char *message) {
	int send_rt;
	char send_line[MAX_LINE];

	sprintf(send_line, "[server] %s", message);

	send_rt = send(conn_fd, send_line, strlen(send_line), 0);
	if (send_rt < 0)
		printf("send failed \n");
	else if (send_rt == 0)
		printf("send failed: client closed \n");

	return send_rt;
}

char *execute_cmd(char *cmd) {
	FILE *file;
	char buf[FILE_MAX_BUFFER], *data, *data_idx;

	data = malloc(16384);
	bzero(data, sizeof(data));
	data_idx = data;
	file = popen(cmd, "r");

	if (!file) return data;

	while (!feof(file)) {
		if (fgets(buf, FILE_MAX_BUFFER, file) == NULL)
			break;
		int len = strlen(buf);
		memcpy(data_idx, buf, len);
		data_idx += len;
	}

	pclose(file);
	return data;
}

int main(int argc, char **argv) {
	int listen_fd, conn_fd;
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	signal(SIGPIPE, SIG_IGN);

	listen_fd = create_tcp_server_socket(SERVER_PORT);

	for (;;) {
		if ((conn_fd = accept_client_connect(listen_fd, &client_addr, &client_len)) < 0)
			continue;

		handle_client_request(conn_fd, client_addr);

		close(conn_fd);
	}
}
