#ifndef __LIST_H__
#define __LIST_H__

struct list_node {
  struct list_node *next;
  struct list_node *prev;
  void *data
};

typedef void (*list_node_data_destructor)(void*);

struct list_node* list_build_node(void *data);

struct slist_node* list_insert(struct list_node *head, struct list_node *new_node);
void list


struct list_node* list_delete(struct list_node *head, struct list_node *node);

void list_destroy(struct list_node *head, list_node_data_destructor destructor);

#endif
