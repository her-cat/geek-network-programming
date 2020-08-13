#include "channel.h"
#include <stdlib.h>

struct channel *channel_create(int fd, int events, void *data, event_read_callback read, event_write_callback write) {
	struct channel *c = malloc(sizeof(struct channel));
	c->fd = fd;
	c->events = events;
	c->data = data;
	c->read = read;
	c->write = write;
	return c;
}

int channel_write_event_is_enabled(struct channel *c) {
	return c->events & EVENT_WRITE;
}

