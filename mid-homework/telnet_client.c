#include "./msg_obj.h"
#include "../lib/common.h"
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_PORT 8017
#define KEEP_ALIVE_TIME 10
#define KEEP_ALIVE_INTERVAL 3
#define KEEP_ALIVE_PROBE_TIMES 3

int connect_server(const char *ip_addr, const int port);
int read_message(int socket_fd, struct msg_obj *msg, size_t size);
int send_message(int socket_fd, struct msg_obj *msg, size_t size);

int connect_server(const char *ip_addr, const int port) {
    int socket_fd;

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error(1, errno, "create socket failed");
    }

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip_addr, &server_addr.sin_addr);

    if (connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        error(1, errno, "connect failed");
    }

    return socket_fd;
}

int read_message(int socket_fd, struct msg_obj *msg, size_t size) {
    int recv_rt = read(socket_fd, (char *) msg, size);
	if (recv_rt < 0)
		printf("read failed \n");
	else if (recv_rt == 0)
		printf("server terminated \n");

	return recv_rt;
}

int send_message(int socket_fd, struct msg_obj *msg, size_t size) {
	int send_rt;

	send_rt = send(socket_fd, (char *) msg, size, 0);
	if (send_rt < 0)
		printf("send failed \n");
	else if (send_rt == 0)
		printf("server terminated \n");

	return send_rt;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        error(1, 0, "usage: telnet_client <IP address> \n");
    }

    int socket_fd, rt, heartbeats = 0;
    char send_line[MAX_LINE], recv_line[MAX_LINE];
    fd_set read_fds, read_mask;
    struct timeval tv;
    struct msg_obj msg;

    socket_fd = connect_server(argv[1], SERVER_PORT);

    FD_ZERO(&read_fds);
    FD_SET(0, &read_fds);
    FD_SET(socket_fd, &read_fds);

    tv.tv_sec = KEEP_ALIVE_TIME;
    tv.tv_usec = 0;

    while (1) {
        read_mask = read_fds;
        bzero(&send_line, strlen(send_line));
        bzero(&recv_line, strlen(recv_line));

        rt = select(socket_fd + 1, &read_mask, NULL, NULL, &tv);
        if (rt < 0) {
            error(1, errno, "select failed");
        } else if (rt == 0) {
            if (++heartbeats > KEEP_ALIVE_PROBE_TIMES)
                error(1, 0, "connection dead \n");

            msg.type = htonl(MSG_PING);
            strcpy(msg.data, "testdata");
            printf("send data: %s \n", msg.data);
            if (send_message(socket_fd, &msg, sizeof(msg)) == 0)
                break;

            tv.tv_sec = KEEP_ALIVE_INTERVAL;
            continue;
        }

        heartbeats = 0;
        tv.tv_sec = KEEP_ALIVE_TIME;

        if (FD_ISSET(socket_fd, &read_mask)) {
            if (read_message(socket_fd, &msg, sizeof(msg)) == 0)
                break;

            switch (ntohl(msg.type)) {
                case MSG_PONG:
                    printf("received server pong. \n");
                    break;
                case MSG_TEXT:
                    fputs(msg.data, stdout);
                    fputs("\n", stdout);
                default:
                    printf("unknow message type.");
                    break;
            }
        }

        if (FD_ISSET(0, &read_mask)) {
            if(fgets(msg.data, MAX_LINE, stdin) == NULL)
                continue;

            msg.type = htonl(MSG_TEXT);
            if (send_message(socket_fd, &msg, sizeof(msg)) == 0)
                break;
        }
    }

    return EXIT_SUCCESS;
}
