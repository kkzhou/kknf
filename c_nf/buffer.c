#include "buffer.h"

#define FRAG_SIZE_BASE 1024
#define FRAG_NUM_MAX 10
#define FRAG_INCR_FACTOR 2

struct buffer* buffer_init()
{
  struct buffer *ret = (struct buffer*)malloc(sizezof (struct buffer));
  ret->frag_vector = (struct fragment*)malloc(FRAG_NUM_MAX * sizeof (void*));
  memset(ret->frag_vector, FRAG_NUM_MAX * sizeof (void*), 0);
  ret->vector_length = 0;
  ret->inuse_size = ret->total_size = 0;
  ret->cur_item = ret->cur_pos = 0;
  ret->end_item = ret->end_pos = 0;
  return ret;
}

int buffer_enlarge(struct buffer const* buffer, uint32_t size)
{

  if (buffer->vector_length == FRAG_NUM_MAX) {
    return -1;
  }

  uint32_t delta = (buffer->vector_length + FRAG_INCR_FACTOR) * FRAG_SIZE_BASE;
  if (delta < size) {
    delta = size;
  }

  struct framgment *frag = (struct fragment*)malloc(sizeof (struct fragment) + delta);
  frag->size = delta;
  frag->end = 0;

  buffer->vector_length++;
  buffer->frag_vector[end_item] = frag;

  buffer->end_item++;
  buffer->end_pos = frag->end;
  buffer->total_size += frag->size;
  return 0;
}

void buffer_destroy(struct buffer *buffer)
{
  assert(buffer);
  for (uint32_t i = 0; i < FRAG_NUM_MAX; i++) {
    free(buffer->frag_vector[i]);
  }
  free(buffer->frag_vector);
  free(buffer);
}

int buffer_forward_cur(struct buffer const *buffer, uint32_t offset)
{
  assert(buffer);
  assert(buffer->frag_vector);

  uint32_t len, cur_item, cur_pos, cur_end;
  len = 0;
  cur_item = buffer->cur_item;

  while (len < offset) {
    if (cur_item == FRAG_NUM_MAX)
      break;
    cur_pos = buffer->cur_pos;
    cur_end = buffer->frag_vector[cur_item]->end;
    if (offset - len < cur_end - cur_pos) {
      /* the current node meets the demand */
      cur_pos += offset - len;
      len += offset -len;
      break;
    }
    len += cur_end - cur_pos;
    cur_item++;
    cur_pos = 0;
  }

  if (len == offset) {
    buffer->cur_item = cur_item;
    buffer->cur_pos = cur_pos;
    return len;
  }
  return 0;
}

void* buffer_alloc(struct buffer *buffer, unint32_t size)
{
  assert(buffer);
  assert(buffer->frag_vector);
  struct fragment *frag = buffer->frag_vector[buffer->end_item];
  if (frag->size - frag->end < size) {
    /* enlarge */
    if (buffer_enlarge(buffer, size) != 0) {
      return 0;
    }
  }

  frag = buffer->frag_vector[buffer->end_item];
  frag->end += size;
  buffer->end_pos = frag->end;
  
  return frag->data;
}

struct iovec* buffer_get_used(struct buffer *buffer)
{
  assert(buffer);
  
}
