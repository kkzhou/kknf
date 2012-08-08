#include "buffer.h"
struct buffer* buffer_init()
{
  struct buffer *ret = (struct buffer*)malloc(sizezof (struct buffer));
  ret->head = slist_init();
  ret->list_size - 0;
  ret->begin_item = ret->begin_pos = 0;
  ret->end_item = ret->end_pos = 0;
  return ret;
}

void buffer_copy_to_append(struct buffer const* const head, char *data, uint32_t data_size)
{
  assert(data);
  assert(data_size);
  struct framgment *new_frag = (struct fragment*)malloc(sizeof (struct fragment) + data_size);
  slist_
}
