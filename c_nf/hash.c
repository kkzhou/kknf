#include "hash.h"

struct hash_item {
  void *key;
  uint32_t key_len;
  struct hash_item *next;
};

struct hash_table {
  uint64_t key_space;
  uint64_t key_used;
  struct hash_item **items;
};

uint64_t ip_port_hash_func(void *key, uint32_t key_len, uint64_t key_space)
{
  
}

void* get_hash_table(uint64_t key_space)
{
  struct hash_table *t = (struct hash_table*)malloc(sizeof (struct hash_table));
  t->key_space = key_space;
  t->key_used = 0;
  t->items = (struct hash_item**)malloc(sizeof (struct hash_table*) * key_space);
  memset(t->items, sizeof (struct hash_table*) * key_space, 0);

  return t;
}

