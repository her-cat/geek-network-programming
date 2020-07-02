#include "dict.h"

/* -------------------------- private prototypes ---------------------------- */

int _dictInit(dict *d, unsigned long size);
unsigned int _dictKeyIndex(dict *d, const void *key);
unsigned int _dictGenHash(const unsigned char *buff, int len);

/* ----------------------------- API implementation ------------------------- */

unsigned int _dictGenHash(const unsigned char *buff, int len) {
	unsigned int hash = 5381;

	while (len--)
		hash = ((hash << 5) + hash) + *buff++;

	return hash;
}

dict *dictCreate(unsigned long size) {
	dict *d = malloc(sizeof(dict));

	assert(d != NULL);

	_dictInit(d, size);

	return d;
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
