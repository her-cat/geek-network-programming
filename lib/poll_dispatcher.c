#include "event_dispatcher.h"
#include "event_loop.h"
#include "common.h"
#include <sys/poll.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

#define POLL_INITIAL_SIZE 1024

struct poll_dispatcher_data {
    int count;
    int nfds;
    int realloc_copy;
    struct pollfd *fds;
    struct pollfd *rfds;
};

static void *poll_init(struct event_loop *event_loop);
static int poll_add(struct event_loop *event_loop, struct channel *c);
static int poll_del(struct event_loop *event_loop, struct channel *c);
static int poll_update(struct event_loop *event_loop, struct channel *c);
static int poll_dispatch(struct event_loop *event_loop, struct timeval *);
static void poll_clear(struct event_loop * event_loop);

const struct event_dispatcher poll_dispatcher = {
    "poll",
    poll_init,
    poll_add,
    poll_del,
    poll_update,
    poll_dispatch,
    poll_clear
};

void *poll_init(struct event_loop *event_loop) {
    struct poll_dispatcher_data *data;

    data = malloc(sizeof(struct poll_dispatcher_data));
    data->fds = malloc(sizeof(struct pollfd) * POLL_INITIAL_SIZE);

    for (int i = 0; i < POLL_INITIAL_SIZE; i++)
        data->fds[i].fd = -1; /* 用 -1 表示未被使用 */

    data->count = 0;
    data->nfds = 0;
    data->realloc_copy = 0;

    return data;
}

int poll_add(struct event_loop *event_loop, struct channel *c) {
    int i = 0, events;
    struct poll_dispatcher_data *data;

    events = 0;
    data = (struct poll_dispatcher_data *) event_loop->data;

    if (c->events && EVENT_READ)
        events |= POLLRDNORM;
    if (c->events && EVENT_WRITE)
        events |= POLLWRNORM;

    for (i = 0; i < POLL_INITIAL_SIZE; i++) {
        if (data->fds[i].fd < 0) {
            data->fds[i].fd = c->fd;
            data->fds[i].events = events;
            break;
        }
    }

    printf("poll added channel fd==%d, %s \n", c->fd, event_loop->thread_name);

    if (i >= POLL_INITIAL_SIZE)
        printf("too many clients, just abort it \n");

    return 0;
}

int poll_del(struct event_loop *event_loop, struct channel *c) {
    int i = 0;
    struct poll_dispatcher_data *data;

    data = (struct poll_dispatcher_data *) event_loop->data;

    for (i = 0; i < POLL_INITIAL_SIZE; i++) {
        if (data->fds[i].fd == c->fd) {
            data->fds[i].events = -1;
            break;
        }
    }
    
    printf("poll deleted channel fd==%d, %s \n", c->fd, event_loop->thread_name);

    if (i >= POLL_INITIAL_SIZE)
        printf("can not find fd, poll delete error it \n");
    
    return 0;
}

int poll_update(struct event_loop *event_loop, struct channel *c) {
    int i = 0, events;
    struct poll_dispatcher_data *data;

    data = (struct poll_dispatcher_data *) event_loop->data;

    if (c->events && EVENT_READ)
        events |= POLLRDNORM;
    if (c->events && EVENT_WRITE)
        events |= POLLWRNORM;

    for (i = 0; i < POLL_INITIAL_SIZE; i++) {
        if (data->fds[i].fd == c->fd) {
            data->fds[i].events = events;
            break;
        }
    }
    
    printf("poll updated channel fd==%d, %s \n", c->fd, event_loop->thread_name);

    if (i >= POLL_INITIAL_SIZE)
        printf("can not find fd, poll update error it \n");
    
    return 0;
}

int poll_dispatch(struct event_loop *event_loop, struct timeval *timeval) {
    int ready_num, timeout;
    struct poll_dispatcher_data *data;

    ready_num = 0;
    timeout = timeval->tv_sec * 1000;
    data = (struct poll_dispatcher_data *) event_loop->data;

    if ((ready_num = poll(data->fds, POLL_INITIAL_SIZE, timeout)) < 0)
        error(1, errno, "poll error");

    if (ready_num <= 0)
        return 0;

    for (int i = 0; i < POLL_INITIAL_SIZE; i++) {
        if (data->fds[i].fd < 0 || data->fds[i].revents <= 0)
            continue;

        printf("get message channel i==%d fd==%d, %s \n", i, data->fds[i].fd, event_loop->thread_name);

        int events = 0;
        if (data->fds[i].revents & POLLRDNORM)
            events |= EVENT_READ;
        if (data->fds[i].revents & POLLWRNORM)
            events |= EVENT_WRITE;
        
        if (events > 0)
            channel_event_activate(event_loop, data->fds[i].fd, events);

        if (--ready_num <= 0)
            break;
    }

    return 0;
}

void poll_clear(struct event_loop * event_loop) {
    struct poll_dispatcher_data *data;

    data = (struct poll_dispatcher_data *) event_loop->data;

    free(data->fds);
    data->fds = NULL;
    free(data);
    event_loop->data = NULL;
}
