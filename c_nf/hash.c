#include "hash.h"

struct hash_item {
  void *key;
  uint32_t key_len;
  void *value;
  struct hash_item *next;
};

struct hash_table {
  uint64_t key_space;
  uint64_t key_used;
  hash_func hash;
  struct hash_item **items;
};

uint64_t ip_port_hash(void *key, uint32_t key_len, uint64_t key_space)
{
  uint64_t ret = *((uint64_t*)key);
  return ret % key_space;
}

void* hash_get_hash_table(uint64_t key_space, hash_func f = 0)
{
  struct hash_table *t = (struct hash_table*)malloc(sizeof (struct hash_table));
  t->key_space = key_space;
  t->key_used = 0;
  if (!f) {
    t->hash = ip_port_hash;
  } else {
    t->hash = f;
  }

  t->items = (struct hash_item**)malloc(sizeof (struct hash_item*) * key_space);
  memset(t->items, sizeof (struct hash_table*) * key_space, 0);
    
  return t;
}

void* hash_get(void *t, void *key, uint32_t key_len) 
{
  assert(t);
  assert(key);

  struct hash_table *table = (struct hash_table*)t;
  uint64_t h = table->hash(key, key_len, table->key_space);
  struct hash_item *cur = table->items[h];

  if (!cur) {
    return 0;
  }

  do {
    /* there is a chain */
    if (memcmp(key, cur->key, key_len) == 0) {
      break;
    }
    cur = cur->next;
  } while (cur);
  
  return cur->value;
}

int hash_put(void *t, void *key, uint32_t key_len, void *value, uint32_t value_len)
{
  assert(t);
  assert(key);
  assert(value);

  struct hash_table *table = (struct hash_table*)t;
  uint64_t h = table->hash(key, key_len, table->key_space);

  struct hash_item *new_item =  (struct hash_item*)malloc(sizeof (struct hash_item));
  
  new_item->key = key;
  new_item->key_len = key_len;
  new_item->value = value;

  if (table->items[h]) {
    assert (!(table->items[h]->prev));
    table->items[h]->prev = new_item;
    new_item->next = table->items[h];
    new_item->prev = 0;
    table->items[h] = new_item;
    
  } else {
    
    table->items[h] = new_item;
   
  }
  table->key_used++;
  return (table->key_sapce >= table->key_used) ? -1 : 0;
}

void* hash_delete(void *t, void *key, uint32_t key_len)
{
  assert(t);
  assert(key);

  struct hash_table *table = (struct hash_table*)t;
  uint64_t h = table->hash(key, key_len, table->key_space);
  struct hash_item *cur = table->items[h];

  if (!cur) {
    return 0;
  }

  do {
    /* there is a chain */
    if (memcmp(key, cur->key, key_len) == 0) {
      break;
    }
    cur = cur->next;
  } while (cur);
  
  if (cur->prev) {
    cur->prev->next = cur->next;
  } else {
    /* header */
    table->items[h] = cur->next;
  }

  if (cur->next) {
    cur->next->prev = cur->prev;
  } else {
  }

  free(cur->key);
  void *ret = cur->value;
  free(cur);
  return ret;
  
}
