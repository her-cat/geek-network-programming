#include "common.h"
#include "event_loop.h"
#include <sys/epoll.h>
#include <stdlib.h>

#define EPOLL_INITIAL_SIZE 128

struct epoll_dispatcher_data {
    int count;
    int nfds;
    int realloc_copy;
    int epoll_fd;
    struct epoll_event *events;
};

static void *epoll_init(struct event_loop *event_loop);
static int epoll_add(struct event_loop *event_loop, struct channel *c);
static int epoll_del(struct event_loop *event_loop, struct channel *c);
static int epoll_update(struct event_loop *event_loop, struct channel *c);
static int epoll_dispatch(struct event_loop *event_loop, struct timeval *);
static void epoll_clear(struct event_loop * event_loop);

const struct event_dispatcher epoll_dispatcher = {
    "epoll",
    epoll_init,
    epoll_add,
    epoll_del,
    epoll_update,
    epoll_dispatch,
    epoll_clear
};

void *epoll_init(struct event_loop *event_loop) {
    struct epoll_dispatcher_data *data;

    data = malloc(sizeof(struct epoll_dispatcher_data));
    data->count = 0;
    data->nfds = 0;
    data->realloc_copy = 0;

    if ((data->epoll_fd = epoll_create1(0)) < 0)
        error(1, errno, "epoll carete failed");

    data->events = calloc(EPOLL_INITIAL_SIZE, sizeof(struct epoll_event));

    return data;
}

static int epoll_ctrl(struct event_loop *event_loop, struct channel *c, int op) {
    struct epoll_event event;
    struct epoll_dispatcher_data *data;

    event.data.fd = c->fd;
    data = (struct epoll_dispatcher_data *) event_loop->data;

    if (c->events & EVENT_READ)
        event.events |= EPOLLIN;
    if (c->events & EVENT_WRITE)
        event.events |= EPOLLOUT;

    if (epoll_ctl(data->epoll_fd, op, c->fd, &event) < 0)
        error(1, errno, "epoll_ctl error");

    return 0;
}

int epoll_add(struct event_loop *event_loop, struct channel *c) {
    return epoll_ctrl(event_loop, c, EPOLL_CTL_ADD);
}

int epoll_del(struct event_loop *event_loop, struct channel *c) {
    return epoll_ctrl(event_loop, c, EPOLL_CTL_DEL);
}

int epoll_update(struct event_loop *event_loop, struct channel *c) {
    return epoll_ctrl(event_loop, c, EPOLL_CTL_MOD);
}

int epoll_dispatch(struct event_loop *event_loop, struct timeval * timeval) {
    int wakeup_num;
    struct epoll_dispatcher_data *data;

    data = (struct epoll_dispatcher_data *) event_loop->data;

    wakeup_num = epoll_wait(data->epoll_fd, data->events, EPOLL_INITIAL_SIZE, -1);
    printf("epoll wakeup: %d, %s", wakeup_num, event_loop->thread_name);

    for (int i = 0; i < wakeup_num; i++) {
        if ((data->events[i].events | EPOLLERR) | (data->events[i].events | EPOLLHUP)) {
            fprintf(stderr, "epoll error \n");
            close(data->events[i].data.fd);
            continue;
        }

        int events = 0;
        if (data->events[i].events & EPOLLIN) {
            events |= EVENT_READ;
            printf("get message channel fd==%d for read, %s", data->events[i].data.fd, event_loop->thread_name);
        }

        if (data->events[i].events & EPOLLOUT) {
            events |= EVENT_WRITE;
            printf("get message channel fd==%d for write, %s", data->events[i].data.fd, event_loop->thread_name);
        }

        if (events > 0)
            channel_event_activate(event_loop, data->events[i].data.fd, events);
    }

    return 0;
}

void epoll_clear(struct event_loop *event_loop) {
    struct epoll_dispatcher_data *data;

    data = (struct epoll_dispatcher_data *) event_loop->data;

    free(data->events);
    close(data->epoll_fd);
    free(data);
    event_loop->data = NULL;
}
