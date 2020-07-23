#ifndef __DICT_H__
#define __DICT_H__

#include <stdlib.h>

#define DICT_OK 1
#define DICT_ERR 0

#define DICT_HT_INITIAL_SIZE 4

#define dictSize(d) ((d)->ht[0].used + (d)->ht[1].used)
#define dictIsRehashing(d) ((d)->rehashidx != -1)

typedef struct dictEntry {
	void *key;
	void *val;
	struct dictEntry *next;
} dictEntry;

typedef struct dictht {
	dictEntry **table;
	unsigned long size;
	unsigned long mask;
	unsigned long used;
} dictht;

typedef struct dict {
	dictht ht[2];
	long rehashidx;
} dict;

dict *dictCreate(unsigned long size);
int dictAdd(dict *d, void *key, void *val);
dictEntry *dictGet(dict *d, void *key);
dictEntry *dictDel(dict *d, void *key);
int dictExpand(dict *d, unsigned long size);
int dictRehash(dict *d, int n);

#endif
