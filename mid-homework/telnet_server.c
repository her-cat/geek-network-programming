#include "./msg_obj.h"
#include "../lib/common.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#define SERVER_PORT 8017
#define BACKLOG 128
#define FILE_MAX_BUFFER 256

int create_tcp_server_socket(int port);
int accept_client_connect(int listen_fd, struct sockaddr_in *client_addr, socklen_t *client_len);
void handle_client_request(int conn_fd, struct sockaddr_in client_addr);
int read_client_message(int conn_fd, struct msg_obj *msg, size_t size);
int reply_client_message(int conn_fd, struct msg_obj *msg, size_t size);
char *execute_system_cmd(char *cmd);
int execute_cmd(int conn_fd, char *cmd);

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
	struct msg_obj msg;

	printf("%s:%d connected... \n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

	for (;;) {
		recv_rt = read_client_message(conn_fd, &msg, sizeof(msg));

		if (recv_rt < 0)
			continue;
		else if (recv_rt == 0)
			break;
		
		printf("msg_type:%d \n", ntohl(msg.type));
		
		switch (ntohl(msg.type)) {
            case MSG_PING:
                printf("received server pong. \n");
                break;
            case MSG_TEXT:
				if (execute_cmd(conn_fd, msg.data) < 0)
					break;
            default:
                printf("unknow message type. \n");
                break;
        }

		printf("received %d bytes:%s \n", recv_rt - 1, msg.data);
	}
}

int read_client_message(int conn_fd, struct msg_obj *msg, size_t size) {
	int recv_rt = read(conn_fd, (char *) msg, size);
	if (recv_rt < 0)
		printf("read failed \n");
	else if (recv_rt == 0)
		printf("client closed \n");

	return recv_rt;
}

int reply_client_message(int conn_fd, struct msg_obj *msg, size_t size) {
	int send_rt;

	send_rt = send(conn_fd, (char *) msg, size, 0);
	if (send_rt < 0)
		printf("send failed \n");
	else if (send_rt == 0)
		printf("send failed: client closed \n");

	return send_rt;
}

char *execute_system_cmd(char *cmd) {
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

int execute_cmd(int conn_fd, char *cmd) {
	struct msg_obj msg;
	msg.type = htonl(MSG_TEXT);

	if (strncmp(cmd, "quit", 4) == 0) {
		strcpy(msg.data, "good bye!");
		reply_client_message(conn_fd, &msg, sizeof(msg));
		printf("client quit \n");
		return -1;
	} else if (strncmp(cmd, "ls", 2) == 0) {
		strcpy(msg.data, execute_system_cmd("ls"));
	} else if (strncmp(cmd, "pwd", 3) == 0) {
		strcpy(msg.data, getcwd(NULL, 0));
	} else if (strncmp(cmd, "cd", 2) == 0) {
		char path[256];
		bzero(path, sizeof(path));
		// + 3:排除 cd 及空格，- 4: 排除 cd、空格及 \r\n 的长度
		memcpy(path, cmd + 3, strlen(cmd) - 4);
		if (chdir(path) < 0)
			strcpy(msg.data, "change dir failed");
	} else {
		strcpy(msg.data, "unknow command");
	}

	if (reply_client_message(conn_fd, &msg, sizeof(msg)) == 0)
		return -1;

	return 1;
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
