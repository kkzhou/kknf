#ifndef __SLIST_H__
#define __SLIST_H__
/*
  callback function used to compare two elements in a slist
  return value:
  0: equal
  -1: less than
  1: larger than
  other: invalid
 */
typedef int (*slist_node_compare)(void*, void*);

struct slist_node {
  struct slist_node *next;
  struct slist_node *prev;
  void *data
};

struct slist_head {
  struct slist_node guard;
  uint32_t len;
};

struct slist_head* slist_init();

void slist_insert(struct slist_head const *const head, struct slist_node *new_node, slist_node_compare cmp);

struct slist_node* slist_remove(struct slist_head const *const head, void const *const value, 
                                slist_node_compare cmp);

void slist_erase(struct slist_head const *const head, struct slist_node *node);

struct slist_node* slist_first(struct slist_head const *const head);

void* slist_pop(struct slist_head const *const head);

#endif
