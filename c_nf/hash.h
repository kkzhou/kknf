#ifndef __HASH_H__
#define __HASH_H__

void* get_hash_table(uint64_t key_space);
void* get_item(void *key, uint32_t key_len);
int put_item(void *key, uint32_t key_len);
void delete_item(void *key, uint32_t key_len);

#endif
