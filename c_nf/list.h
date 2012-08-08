#ifndef __LIST_H__
#define __LIST_H__


struct list_node {
  struct list_node *next;
  struct list_node *prev;
  void *data
};

struct slist_node* list_init();

void list_insert(struct list_node const *const head, struct list_node *new_node);

void list_erase(struct list_node const *const head, struct list_node *node);

struct list_node* list_first(struct list_node const *const head);

void* list_pop(struct list_head const *const head);

#endif
