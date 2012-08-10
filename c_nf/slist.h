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

typedef void (*list_node_data_destructor)(void*);

struct slist_node* slist_build_node(void *data);

struct slist_node* slist_insert(struct slist_node *head, struct slist_node *new_node, slist_node_compare cmp);

struct slist_node* slist_insert_after(struct slist_node *node, struct slist_node *new_node, slist_node_compare cmp);

struct slist_node* slist_remove(struct slist_node *head, void *value, slist_node_compare cmp);

struct slit_node* slist_delete(struct slist_node *head, struct slist_node *node);

void list_destroy(struct list_node *head, list_node_data_destructor destructor);

#endif
