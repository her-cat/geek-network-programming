#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__

struct acceptor {
    int fd; /* 监听的套接字 */
    int port; /* 监听的端口 */
};

struct acceptor *acceptor_init(int port);

#endif
