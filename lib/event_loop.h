#ifndef __EVENT_LOOP_H__
#define __EVENT_LOOP_H__

#include "channel.h"
#include "channel_map.h"
#include "event_dispatcher.h"
#include <pthread.h>

#define CHANNEL_ELEMENT_ADD 1
#define CHANNEL_ELEMENT_DEL 2

extern const struct event_dispatcher poll_dispatcher;
extern const struct event_dispatcher epoll_dispatcher;

struct channel_element {
    int type; /* 1:add 2:delete */
    struct channel *channel;
    struct channel_element *next;
};

struct event_loop {
    int quit;
    const struct event_dispatcher *dispatcher;

    /* 对应的 event_dispatcher 的数据 */
    void *data;
    int is_handle_pending;
    struct channel_map *channel_map;
    struct channel_element *pending_head;
    struct channel_element *pending_tail;

    pthread_t owner_thread_id;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int socket_pair[2];
    char *thread_name;
};

struct event_loop *event_loop_init();
struct event_loop *event_loop_init_with_name(char *thread_name);
void event_loop_wakeup(struct event_loop *event_loop);
int channel_event_activate(struct event_loop *eventLoop, int fd, int events);

#endif
