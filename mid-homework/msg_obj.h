#ifndef __MSG_OBJ_H__
#define __MSG_OBJ_H__

#include <sys/types.h>

#define MSG_PING 1
#define MSG_PONG 2
#define MSG_TEXT 11
#define MAX_LINE 4096

struct msg_obj {
	u_int32_t type;
	char data[MAX_LINE];
};

#endif
