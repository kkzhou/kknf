#ifndef __BUFFER_H__
#define  __BUFFER_H__

#include "slist.h"

struct fragment {
  uint32_t size;
  uint32_t tail;
  char data[0];
};

struct buffer {
  struct slist_node *head;
  uint32_t list_size;
  uint32_t begin_item;
  uint32_t begin_pos;
  uint32_t end_item;
  uint32_t end_pos;
};

struct buffer* buffer_init();

#endif
