#include "buffer.h"
struct buffer* buffer_init()
{
  struct buffer *ret = (struct buffer*)malloc(sizezof (struct buffer));
  ret->head = ret->tail = 0;
  ret->list_length = ret->buffer_inuse_size = ret->buffer_total_size = 0;
  ret->cur_item = ret->cur_pos = 0;
  ret->end_item = ret->end_pos = 0;
  return ret;
}

void buffer_append(struct buffer const* buffer, struct fragment *frag)
{
  assert(frag);

  struct list_node *new_node = (struct list_node*)malloc(sizeof (struct list_node));
  new_node->next = new_node->prev = 0;
  new_node->data = new_frag;
  if (buffer->tail){
    list_insert_after(buffer->tail, new_node, 0);
  } else {
    assert(!buffer->head);
    buffer->head = buffer->tail = new_node;
  }

  buffer->end_item = buffer->tail;
  buffer->end_pos = frag->end;
  buffer->list_length++;
  buffer->buffer_inuse_size += buffer->end_pos;
  buffer->buffer_total_size += buffer->size;
}

void buffer_destroy(struct buffer *buffer)
{
  assert(buffer);
  list_destroy(buffer->head);
  free(buffer);
}

int buffer_forward_cur(struct buffer const *buffer, uint32_t offset)
{
  assert(buffer);
  assert(buffer->head);

  uint32_t len = 0;
  struct slist_node *cur_item = buffer->cur_item;
  uint32_t cur_pos = buffer->cur_pos;
  uint32_t cur_size;

  while (len < offset) {
    if (!cur_item)
      break;

    if (offset - len < cur_size - cur_pos) {
      /* the current node meets the demand */
      cur_pos += offset - len;
      len += offset -len;
      break;
    } 
    len += cur_size - cur_pos;
    cur_item = cur_item->next;
    cur_pos = cur_item->end;
  }

  if (len == offset) {
    buffer->cur_item = cur_item;
    buffer->cur_pos = cur_pos;
    return len;
  }
  return 0;
}
