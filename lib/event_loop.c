#include "event_loop.h"
#include "channel_map.h"
#include <pthread.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <stdio.h>

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

#ifdef EPOLL_ENABLE
    /* TODO: implement epoll_dispatcher */
    /* event_loop->dispatcher = &epoll_dispatcher; */
#else
    /* TODO: implement poll_dispatcher */
    /* event_loop->dispatcher = &poll_dispatcher; */
#endif

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
