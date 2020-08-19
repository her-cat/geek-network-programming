#ifndef __CHANNEL_MAP_H__
#define __CHANNEL_MAP_H__

#define CHANNEL_MAP_DEFAULT_ENTRIES_SIZE 32

/* channel 映射表 */
struct channel_map {
    void **entries; /* key 为 fd，value 为对应的 channel */
    int available_num; /* entries 中可用的数量 */
};

int map_make_space(struct channel_map *map, int slot, int entry_size);
void map_init(struct channel_map *map);
void map_clear(struct channel_map *map);

#endif
