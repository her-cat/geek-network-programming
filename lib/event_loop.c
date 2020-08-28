#include "event_loop.h"
#include "channel_map.h"
#include <pthread.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>

struct event_loop *event_loop_init() {
    return event_loop_init_with_name(NULL);
}

struct event_loop *event_loop_init_with_name(char *thread_name) {
    struct event_loop *event_loop = malloc(sizeof(struct event_loop));

    pthread_mutex_init(&event_loop->mutex, NULL);
    pthread_cond_init(&event_loop->cond, NULL);

    event_loop->quit = 0;
    event_loop->channel_map = malloc(sizeof(struct channel_map));
    event_loop->thread_name = thread_name != NULL ? thread_name : "main thread";

#ifdef HAVE_EPOLL
    event_loop->dispatcher = &epoll_dispatcher;
#else
    #ifdef HAVE_POLL
        event_loop->dispatcher = &poll_dispatcher;
    #else
        /* TODO: Implement select_dispatcher. */
    #endif
#endif

    printf("used %s dispatcher, %s\n", event_loop->dispatcher->name, event_loop->thread_name);

    event_loop->owner_thread_id = pthread_self();
    event_loop->data = (void *)event_loop->dispatcher->init(event_loop);

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, event_loop->socket_pair) < 0) {
        printf("socketpair set failed \n");
    }

    event_loop->is_handle_pending = 0;
    event_loop->pending_head = NULL;
    event_loop->pending_tail = NULL;

    /* TODO: add the socketfd to event */

    return event_loop;
}

void event_loop_wakeup(struct event_loop *event_loop) {
    char one = 'a';
    ssize_t n = write(event_loop->socket_pair[0], &one, sizeof(one));
    if (n != sizeof(one))
        printf("wakeup event loop thread failed \n");
}

int channel_event_activate(struct event_loop *event_loop, int fd, int events) {
    struct channel *channel;

    printf("activate channel fd==%d events=%d, %s\n", fd, events, event_loop->thread_name);

    if (fd < 0) return 0;
    else if (fd > event_loop->channel_map->available_num) return -1;

    channel = event_loop->channel_map->entries[fd];
    assert(fd == channel->fd);

    if (events & EVENT_READ)
        channel->read(channel->data);
    if (events & EVENT_WRITE)
        channel->write(channel->data);

    return 0;
}
