#include "dict.h"
#include <string.h>
#include <stdio.h>

/* -------------------------- private prototypes ---------------------------- */

static int _dictInit(dict *d, unsigned long size);
static long _dictKeyIndex(dict *d, const void *key);
static unsigned int _dictGenHash(const unsigned char *buff, int len);

/* ----------------------------- API implementation ------------------------- */

dict *dictCreate(unsigned long size) {
	dict *d = malloc(sizeof(dict));
	if (d == NULL) {
		return NULL;
	}

	_dictInit(d, size);

	return d;
}

int dictAdd(dict *d, void *key, void *val) {
	long index;
	dictEntry *entry;

	if ((index = _dictKeyIndex(d, key)) == -1) {
		return DICT_ERR;
	}

	entry = malloc(sizeof(*entry));
	entry->key = key;
	entry->val = val;
	entry->next = d->table[index];
	d->table[index] = entry;
	d->used++;

	return DICT_OK;
}

dictEntry *dictGet(dict *d, void *key) {
	long index;
	dictEntry *entry;

	index = d->mask & _dictGenHash(key, strlen(key));

	entry = d->table[index];
	while (entry) {
		if(key == entry->key)
			break;
		entry = entry->next;
	}

	return entry;
}

dictEntry *dictDel(dict *d, void *key) {
	long index;
	dictEntry *entry, *prevEntry = NULL;

	index = d->mask & _dictGenHash(key, strlen(key));

	entry = d->table[index];
	while (entry) {
		if(key == entry->key) {
			if (prevEntry)
				prevEntry->next = entry->next;
			else
				d->table[index] = entry->next;
			d->used--;
			free(entry);
			return entry;
		}
		prevEntry = entry;
		entry = entry->next;
	}

	return entry;
}

int _dictInit(dict *d, unsigned long size) {
	d->table = malloc(sizeof(dictEntry *) * size);
	d->size = size;
	d->mask = size - 1;
	d->used = 0;

	for (int i = 0; i < size; ++i) {
		d->table[i] = NULL;
	}

	return DICT_OK;
}

long _dictKeyIndex(dict *d, const void *key) {
	long idx;
	dictEntry *he;

	idx = d->mask & _dictGenHash(key, strlen(key));

	he = d->table[idx];
	while (he) {
		if(key == he->key)
			return -1;
		he = he->next;
	}

	return idx;
}

unsigned int _dictGenHash(const unsigned char *buff, int len) {
	unsigned int hash = 5381;

	while (len--)
		hash = ((hash << 5) + hash) + *buff++;

	return hash;
}

int main(int argc, char **argv) {
	dict *d = dictCreate(10);
	printf("used: %ld \n", d->used);

	printf("---- dictAdd ----\n");
	printf("first add: %d \n", dictAdd(d, "test", "123"));
	printf("same key add: %d \n", dictAdd(d, "test", "123"));

	printf("---- dictGet ----\n");
	printf("get value: %s \n", (char *)dictGet(d, "test")->val);
	printf("not exists key: %d \n", dictGet(d, "test123") == NULL);

	printf("---- dictDel ----\n");
	printf("del exists key: %s \n", (char *)dictDel(d, "test")->val);
	printf("del not exists key: %d \n", dictDel(d, "test") == NULL);
	printf("add after del: %d \n", dictAdd(d, "test", "1234"));
	printf("get new value: %s \n", (char *)dictGet(d, "test")->val);

	return EXIT_SUCCESS;
}
