#include <assert.h>
#include "debug.h"
#include "slist.h"

struct slist_node* slist_build_node(void *data) 
{
  struct slist_node *ret = (struct slist_node*)malloc(sizeof (struct slist_node));
  ret->prev = ret->next = 0;
  ret->data = data;
  return ret;
}

struct slist_node* slist_insert(struct slist_node *head, struct slist_node *new_node, slist_node_compare cmp) 
{
  assert(head);
  assert(head->data);
  assert(new_node);
  assert(new_node->data);

  if (!cmp) {
    new_node->prev = 0;
    new_node->next = head;
    return new_node;
  }

  struct slist_node *ret = head, *old, *cur = head;
  while (cur) {
    if (cmp(cur->data, new_node->data) >= 0) {
      if (cur->prev) {
        cur->prev->next = new_node;
        new_node->prev = cur->prev;
      } else {
        new_node->prev = 0;
        ret = new_node;/* insert at head */
      }
      new_node->next = cur;
      cur->prev = new_node;
      break;
    }
    cur = cur->next;
    old = cur;
  }
  if (!cur) {
    /* insert at tail */
    new_node->prev = old;
    old->next = new_node;
    new_node->next = 0;
  }
  return ret;
}

struct slist_node* slist_insert_after(struct slist_node *node, struct slist_node *new_node, slist_node_compare cmp) 
{
  assert(new_node);
  assert(new_node->data);
  assert(node);

  struct slist_node *ret = node;
  if (!node)
  if (!cmp || !node->next) {
    new_node->next = node->next;
    if (node->next) {
      node->next->prev = new_node;
    }
    node->next = new_node;
    new_node->prev = node;
    return node;
  }
  ret = slist_insert(node->next, new_node, cmp);
  return ret;  
}

struct slist_node* slist_remove(struct slist_node *head, void *value, slist_node_compare cmp)
{
  assert(head);
  assert(value);
  assert(cmp);
  
  struct slist_node *cur, *ret;
  ret = cur = head;
  while (cur) {
    if (cmp(cur->data, value) == 1) {
      if (cur->prev) {
        cur->prev->next = cur->next;
      }
      if (cur->next) {
        cur->next->prev = cur->prev;
      }
      break;
    }
  }
  if (cur == head) {
    ret = 0;
  }
  free(cur);
  return ret;
}

struct slist_node* slist_delete(struct slist_node *head, struct slist_node *node)   
{
  assert(head);
  assert(node);
  
  struct slist_node *ret = head;
  if (node->prev){
    node->prev->next = node->next;
  } else {
    ret = node->next;
  }

  if (node->next) {
    node->next->prev = node->prev;
  }
   
  return ret;
}

void slist_destroy(struct slist_node *head, slist_node_data_destructor destructor)
{
  assert(head);
  struct list_node *tmp, *iter;
  tmp = iter = head;

  while (iter) {
    tmp = iter;
    iter = iter->next;
    destructor(tmp->data);
    free(tmp)
  }
}
