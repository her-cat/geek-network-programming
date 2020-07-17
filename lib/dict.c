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
	dictht *ht;
	dictEntry *entry;

	// if (dictIsRehashing(d)) _dictRehashStep(d);

	if ((index = _dictKeyIndex(d, key)) == -1) {
		return DICT_ERR;
	}

	ht = dictIsRehashing(d) ? &d->ht[1] : &d->ht[0];

	entry = malloc(sizeof(*entry));
	entry->key = key;
	entry->val = val;
	entry->next = ht->table[index];
	ht->table[index] = entry;
	ht->used++;

	return DICT_OK;
}

dictEntry *dictGet(dict *d, void *key) {
	__uint64_t h, idx;
	dictEntry *entry;

	if (dictSize(d) == 0) return NULL;

	// if (dictIsRehashing(d)) _dictRehashStep(d);

	h = _dictGenHash(key, strlen(key));

	for (int i = 0; i < 2; i++) {
		idx = h & d->ht[i].mask;
		entry = d->ht[i].table[idx];
		while (entry) {
			if(strcmp(key, entry->key) == 0)
				return entry;
			entry = entry->next;
		}
		// 如果没有在 rehash，就不需要去 ht[1] 找了
		if (!dictIsRehashing(d)) return NULL;
	}

	return NULL;
}

dictEntry *dictDel(dict *d, void *key) {
	__uint64_t h, idx;
	dictEntry *entry, *prevEntry = NULL;

	if (dictSize(d) == 0) return NULL;

	// if (dictIsRehashing(d)) _dictRehashStep(d);

	h = _dictGenHash(key, strlen(key));

	for (int i = 0; i < 2; i++) {
		idx = h & d->ht[i].mask;
		entry = d->ht[i].table[idx];
		while (entry) {
			if(key == entry->key) {
				if (prevEntry)
					prevEntry->next = entry->next;
				else
					d->ht[i].table[idx] = entry->next;
				d->ht[i].used--;
				free(entry);
				return entry;
			}
			prevEntry = entry;
			entry = entry->next;
		}
		// 如果没有在 rehash，就不需要去 ht[1] 找了
		if (!dictIsRehashing(d)) return NULL;
	}

	return NULL;
}

int _dictInit(dict *d, unsigned long size) {
	for (int i = 0; i < 2; i++) {
		d->ht[i].table = malloc(sizeof(dictEntry *) * size);
		d->ht[i].size = size;
		d->ht[i].mask = size - 1;
		d->ht[i].used = 0;
	}

	d->rehashidx = -1;

	return DICT_OK;
}

long _dictKeyIndex(dict *d, const void *key) {
	long idx;
	dictEntry *he;

	for (int i = 0; i < 2; i++) {
		idx = d->ht[i].mask & _dictGenHash(key, strlen(key));

		he = d->ht[i].table[idx];
		while (he) {
			if(strcmp(key, he->key) == 0)
				return -1;
			he = he->next;
		}
	}

	return idx;
}

unsigned int _dictGenHash(const unsigned char *buff, int len) {
	unsigned int hash = 5381;

	while (len--)
		hash = ((hash << 5) + hash) + *buff++;

	return hash;
}

#ifdef DICT_TEST_MAIN

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

#endif
