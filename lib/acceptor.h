#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__

struct acceptor {
    int fd;
    int port;
};

struct acceptor *acceptor_init(int port);

#endif
