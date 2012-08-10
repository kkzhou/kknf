
#include <assert.h>
#include "debug.h"
#include "list.h"

struct list_node* list_build_node(char *data)
{
  struct list_node *ret = (struct list_node*)malloc(sizeof (struct list_node));
  ret->prev = ret->next = 0;
  ret->data = data;
  return ret;
}

void list_insert(struct list_node *head, struct list_node *new_node)
{
  assert(head);
  assert(new_node);
  assert(new_node->data);

  new_node->next = head->next;
  new_node->prev = head;

  if (new_node->next) {
    new_node->next->prev = new_node;
  }
}

struct list_node* list_delete(struct list_node *head, struct list_node *node)
{
  assert(head);
  assert(node);

  struct list_node *ret = 0;

  if (node->prev) {
    node->prev->next = node->next;
  }
  if (node->next) {
    node->next->prev = node->prev;
  }

  if (head != node) {
    ret = head;
  }
  return ret;
}

void list_destroy(struct list_node *head, list_node_data_destructor destructor)
{
  if (!head) {
    return;
  }

  struct list_node *tmp, *iter;
  tmp = iter = head;

  while (iter) {
    tmp = iter;
    iter = iter->next;
    destructor(tmp->data);
    free(tmp)
  }
  free(head);
}
