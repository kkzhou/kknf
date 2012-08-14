#ifndef __BUFFER_H__
#define  __BUFFER_H__

struct fragment {
  uint32_t size;
  uint32_t end;
  char data[0];
};

struct buffer {
  struct fragment *frag_vector;
  uint32_t vector_length;
  uint32_t inuse_size;
  uint32_t total_size;

  uint32_t cur_item;
  uint32_t cur_pos;
  uint32_t end_item;
  uint32_t end_pos;
};

#endif
