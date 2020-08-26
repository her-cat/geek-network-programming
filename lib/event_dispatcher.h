#ifndef __EVENT_DISPATCHER_H__
#define __EVENT_DISPATCHER_H__

#include "channel.h"
#include "event_loop.h"
#include <sys/time.h>

struct event_loop;

/* 抽象的 event_dispatcher 结构体，对应的实现如select,poll,epoll等I/O复用. */
struct event_dispatcher {
    /* 对应实现的名称 */
    const char *name;

    /* 初始化函数 */
    void *(*init)(struct event_loop *event);

    /* 通知 dispatcher 新增一个 channel 事件 */
    int (*add)(struct event_loop *event, struct channel *c);

    /* 通知 dispatcher 删除一个 channel 事件 */
    int (*del)(struct event_loop *event, struct channel *c);

    /* 通知 dispatcher 更新一个 channel 事件 */
    int (*update)(struct event_loop *event, struct channel *c);

    /* 实现事件分发， 然后调用 event_loop 的 event_activate 方法执行 callback */
    int (*dispatch)(struct event_loop *event, struct timeval *);

    /* 清除数据 */
    void (*clear)(struct event_loop *event);
};

#endif
