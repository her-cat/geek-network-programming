#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#define EVENT_READ    0x01 /* 等待套接字或 fd 变为可读 */
#define EVENT_WRITE   0x02 /* 等待套接字或 fd 变为可写 */
#define EVENT_SIGNAL  0x04 /* 等待 POSIX 信号发出 */
#define EVENT_TIMEOUT 0x08 /* 等待变为超时 */

typedef int (*event_read_callback)(void *data);
typedef int (*event_write_callback)(void *data);

struct channel {
    int fd;
    int events; /* channel 等待的事件*/
    void *data; /* 回调的数据，可能是 event_loop/tcp_server/tcp_connection */
    event_read_callback read;
    event_write_callback write;
};

struct channel *channel_create(int fd, int events, void *data, event_read_callback read, event_write_callback write);
int channel_write_event_is_enabled(struct channel *c);
int channel_write_event_enable(struct channel *c);
int channel_write_event_disable(struct channel *c);

#endif
