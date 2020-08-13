#include "./common.h"
#include "acceptor.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

struct acceptor *acceptor_init(int port) {
    int on = 1;
    struct sockaddr_in server_addr;
    struct acceptor *acceptor;

    acceptor = malloc(sizeof(struct acceptor));
    acceptor->port = port;

    if ((acceptor->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error(1, errno, "create socket failed");
    }

    fcntl(acceptor->fd, F_SETFL, O_NONBLOCK);
    setsockopt(acceptor->fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(acceptor->fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        error(1, errno, "bind failed");
    }

    if (listen(acceptor->fd, 128)) {
        error(1, errno, "listen failed");
    }

    return acceptor;
}
