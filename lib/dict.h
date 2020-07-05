#ifndef __DICT_H__
#define __DICT_H__

#include <stdlib.h>

#define DICT_OK 1
#define DICT_ERR 0

typedef struct dictEntry {
	void *key;
	void *val;
	struct dictEntry *next;
} dictEntry;

typedef struct dict {
	dictEntry **table;
	unsigned long size;
	unsigned long mask;
	unsigned long used;
} dict;

dict *dictCreate(unsigned long size);
int dictAdd(dict *d, void *key, void *val);
dictEntry *dictGet(dict *d, void *key);
dictEntry *dictDel(dict *d, void *key);

#endif
