#include "dict.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

/* -------------------------- private prototypes ---------------------------- */

static int _dictInit(dict *d, unsigned long size);
static long _dictKeyIndex(dict *d, const void *key);
static unsigned int _dictGenHash(const unsigned char *buff, int len);
static void _dictReset(dictht *ht);
static void _dictRehashStep(dict *d);

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

	if (dictIsRehashing(d)) _dictRehashStep(d);

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

	if (dictIsRehashing(d)) _dictRehashStep(d);

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

	if (dictIsRehashing(d)) _dictRehashStep(d);

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

int dictRehash(dict *d, int n) {
	int empty_visits = n * 10;

	if (!dictIsRehashing(d)) return 0;

	while (--n && d->ht[0].used != 0) {
		dictEntry *de, *nextde;

		assert(d->ht[0].size > (unsigned long) d->rehashidx);
		while (d->ht[0].table[d->rehashidx] == NULL) {
			d->rehashidx++;
			if (--empty_visits == 0) return 1;
		}

		de = d->ht[0].table[d->rehashidx];
		while (de) {
			__uint64_t h;

			nextde = de->next;
			h = _dictKeyIndex(d, de->key) & d->ht[0].mask; // 计算 de 放在哪个槽
			de->next = d->ht[1].table[h];	// 将 de 的后继节点指向新的哈希表 table[h] 的第一个节点
			d->ht[1].table[h] = de;	// 将 de 作为哈希表 table[h] 第一个节点
			d->ht[0].used--; // ht[0] 已使用数量减一
			d->ht[1].used++; // ht[1] 已使用数量加一
			de = nextde; // 指针后移，继续循环
		}
		d->ht[0].table[d->rehashidx] = NULL;
		d->rehashidx++;
	}

	/* 如果 ht[0] 已经转移完毕 */
	if (d->ht[0].used == 0) {
		free(d->ht[0].table);	// 释放 ht[0] 的 table
		d->ht[0] = d->ht[1];	// 将 ht[0] 指向 ht[1]
		_dictReset(&d->ht[1]);	// 重置 ht[1]
		d->rehashidx = -1;		// 修改为未进行 rehash 状态
	}

	return 1;
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

static void _dictReset(dictht *ht) {
    ht->table = NULL;
    ht->size = 0;
    ht->mask = 0;
    ht->used = 0;
}

static void _dictRehashStep(dict *d) {
	dictRehash(d, 1);
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
