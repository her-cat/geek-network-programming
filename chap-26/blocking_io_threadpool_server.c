#include "../lib/common.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVER_PORT 8026
#define MAX_LINE 4096
#define THREAD_NUM 10
#define BLOCKING_QUEUE_SIZE 128

typedef struct {
    int size; /* 当前队列中描述符最大个数 */
    int used;  /* 当前队列中描述符个数 */
    int *fd;    /* 描述符数组 */
    int front;  /* 当前队列的头位置 */
    int rear;   /* 当前队列的尾位置 */
    pthread_mutex_t mutex; /* 锁 */
    pthread_cond_t free;   /* 队列空闲条件变量 */
    pthread_cond_t nonempty;   /* 队列非空条件变量 */
} blocking_queue;

/* 初始化队列 */
void blocking_queue_init(blocking_queue *queue, int size) {
    queue->size = size;
    queue->fd = calloc(size, sizeof(int));
    queue->used = queue->front = queue->rear = 0;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->free, NULL);
    pthread_cond_init(&queue->nonempty, NULL);
}

/* 将连接字放入队列 */
void blocking_queue_push(blocking_queue *queue, int fd) {
    /* 先加锁，防止其它线程读写队列 */
    pthread_mutex_lock(&queue->mutex);
    /* 队列已满则等待空闲信号 */
    while (queue->used == queue->size)
        pthread_cond_wait(&queue->free, &queue->mutex);
    /* 将新的 fd 放在队尾 */
    queue->fd[queue->rear] = fd;
    /* 如果已经到了最后，重置队尾位置 */
    if (++queue->rear == queue->size)
        queue->rear = 0;

    queue->used++;
    printf("[queue] push fd: %d \n", fd);

    /* 通知其他线程有新的连接字等待处理 */
    pthread_cond_signal(&queue->nonempty);
    /* 释放锁 */
    pthread_mutex_unlock(&queue->mutex);
}

/* 从队列取出连接字 */
int blocking_queue_pop(blocking_queue *queue) {
    /* 加锁 */
    pthread_mutex_lock(&queue->mutex);
    /* 如果队列为空就一直条件等待，直到有新连接入队列 */
    while (queue->used == 0)
        pthread_cond_wait(&queue->nonempty, &queue->mutex);
    /* 取出队头的连接字 */
    int fd = queue->fd[queue->front];
    /* 如果已经到最后，重置队头位置 */
    if (++queue->front == queue->size)
        queue->front = 0;

    queue->used--;
    printf("[queue] pop fd: %d \n", fd);

    /* 通知其他线程当前队列有空闲 */
    pthread_cond_signal(&queue->free);
    /* 释放锁 */
    pthread_mutex_unlock(&queue->mutex);

    return fd;
}

int create_tcp_server(int port) {
    int listen_fd, on = 1;
    struct sockaddr_in server_addr;

    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error(1, errno, "create socket failed");
    }

    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        error(1, errno, "bind failed");
    }

    if (listen(listen_fd, 128)) {
        error(1, errno, "listen failed");
    }

    return listen_fd;
}

char rot13_char(char c) {
    if ((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M'))
        return c + 13;
    else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
        return c - 13;
    else 
        return c;
}

void connection_loop(int fd) {
    int result;
    pthread_t tid;
    char buf[MAX_LINE];

    tid = pthread_self();

    while (1) {
        bzero(&buf, MAX_LINE);
        result = read(fd, buf, MAX_LINE);
        if (result < 0) {
            error(1, errno, "read error");
        } else if (result == 0) {
            printf("client closed \n");
            close(fd);
            break;
        }

        printf("[tid:%ld] received %d bytes: %s", tid, result, buf);

        for (int i = 0; i < result; i++) {
            buf[i] = rot13_char(buf[i]);
        }
        
        if (write(fd, buf, result) < 0)
            error(1, errno, "write error");
    }
}

void *thread_run(void *arg) {
    int fd;
    pthread_t tid;
    blocking_queue *queue;

    tid = pthread_self();
    queue = (blocking_queue *) arg;

    /* 分离当前线程，在线程终止后能够自动回收相关的线程资源 */
    pthread_detach(tid);

    while (1) {
        fd = blocking_queue_pop(queue);
        printf("[tid:%ld] service to fd: %d \n", tid, fd);
        connection_loop(fd);
    }
}

int main(int argc, char **argv) {
    pthread_t tid;
    blocking_queue queue;
    int listen_fd, thread_num;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    blocking_queue_init(&queue, BLOCKING_QUEUE_SIZE);

    thread_num = argc > 1 ? atoi(argv[1]) : THREAD_NUM;
    for (int i = 0; i < thread_num; i++) {
        pthread_create(&tid, NULL, thread_run, (void *) &queue);
    }

    listen_fd = create_tcp_server(SERVER_PORT);

    printf("TCP Server ON: 0.0.0.0:%d Thread num: %d \n", SERVER_PORT, thread_num);

    while (1) {
        int conn_fd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_len);
        if (conn_fd < 0) {
            error(0, errno, "accept error");
            continue;
        }
        printf("client connected: %s:%d \n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        /* 为每一个连接创建一个线程进行通信 */
        blocking_queue_push(&queue, conn_fd);
    }

    return EXIT_SUCCESS;
}
