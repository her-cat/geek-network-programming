#include "channel_map.h"
#include <stdlib.h>
#include <string.h>

int map_make_space(struct channel_map *map, int slot, int entry_size) {
    int available_num;
    void **entries;

    if (map->available_num > slot)
        return 0;

    available_num = map->available_num ? map->available_num : CHANNEL_MAP_DEFAULT_ENTRIES_SIZE;

    while (available_num <= slot)
        available_num <<= 1;
    
    entries = (void **)realloc(map->entries, available_num * entry_size);
    if (entries == NULL)
        return -1;
    
    memset(&entries[map->available_num], 0, (available_num - map->available_num) * entry_size);

    map->entries = entries;
    map->available_num = available_num;

    return 0;
}

void map_init(struct channel_map *map) {
    map->entries = NULL;
    map->available_num = 0;
}

void map_clear(struct channel_map *map) {
    if (map->entries != NULL) {
        for (int i = 0; i < map->available_num; i++)
            if (map->entries[i] != NULL)
                free(map->entries[i]);
        free(map->entries);
        map->entries = NULL;
    }
    map->available_num = 0;
}
