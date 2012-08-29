#ifndef __HASH_H__
#define __HASH_H__

typedef uint64_t (*hash_func)(char*, uint32_t, uint64_t);

void* get_hash_table(uint64_t key_space, hash_func f = 0);
void* get_item(void *t, void *key, uint32_t key_len);
int put_item(void *t, void *key, uint32_t key_len, void *value, uint32_t value_len);
void* delete_item(void *t, void *key, uint32_t key_len);

#endif
