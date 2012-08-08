#include <assert.h>
#include "debug.h"
#include "slist.h"

struct slist_node* slist_init() 
{
  struct slist_node *h = (struct slist_node*)malloc(sizeof (struct slist_node));
  h->prev = h->next = 0;
  h->data = 0;

  return h;
}

void slist_insert(struct slist_node const *const head, struct slist_node *new_node, slist_node_compare cmp) 
{
  assert(head);
  assert(new_node);
  assert(new_node->data);
  assert(cmp);

  struct slist_node *cur = head->next;
  while (cur) {
    if (cmp(cur->data, new_node->data) >= 0) {
      new_node->next = cur->next;
      new_node->prev = cur->prev;
      if (cur->next) {
        cur->next->prev = new_node;
      }
      break;
    }
    cur = cur->next;
  }
}

struct slist_node* slist_remove(struct slist_node const *const head, void const *const value, 
                                slist_node_compare cmp)
{
  assert(head);
  assert(value);
  assert(cmp);
  
  struct slist_node *cur => head->next;
  while (cur) {
    if (cmp(cur->data, value) == 1) {
      cur->prev->next = cur->next;
      if (cur->next) {
        cur->next->prev = cur->prev;
      }
      break;
    }
  }
  return cur;
}

void slist_erase(struct slist_node const *const head, struct slist_node *node)
{
  assert(head);
  assert(node);
  assert(cmp);
  assert(node->prev);
  assert(head->next != 0);

  node->prev->next = node->next;
  if (node->next) {
    node->next->prev = node->prev;
  }
}

struct slist_node* slist_first(struct slist_node const *const head)
{
  assert(head);
  return head->next;
}

void* slist_pop(struct slist_node const *const head)
{
  assert(head);

  struct slist_node *first = head->next;
  void *ret = 0;
  if (first) {
    slist_erase(head, first);
    ret = first->data;
    free(first);
  }
  return ret;
}
