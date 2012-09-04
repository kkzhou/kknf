#ifndef __HASH_H__
#define __HASH_H__

#include <inttypes.h>

typedef uint64_t (*hash_func)(char*, uint32_t, uint64_t);

void* hash_get_hash_table(uint64_t key_space, hash_func f = 0);
void* hash_get(void *t, void *key, uint32_t key_len);
int hash_put(void *t, void *key, uint32_t key_len, void *value, uint32_t value_len);
void* hash_delete(void *t, void *key, uint32_t key_len);

#endif
