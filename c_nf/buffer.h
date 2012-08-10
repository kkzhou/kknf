#ifndef __BUFFER_H__
#define  __BUFFER_H__

#include "slist.h"

struct fragment {
  uint32_t size;
  uint32_t end;
  char data[0];
};

struct buffer {
  struct slist_node *head;
  struct slist_node *tail;
  uint32_t list_length;
  uint32_t buffer_inuse_size;
  uint32_t buffer_total_size;

  struct slist_node *cur_item;
  uint32_t cur_pos;
  struct slist_node *end_item;
  uint32_t end_pos;
};

struct buffer* buffer_init();

#endif
