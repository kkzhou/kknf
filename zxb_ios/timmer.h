#ifndef __TIMER_H__
#define __TIMER_H__

typdef int (*timer_handler)(void*, uint32_t);

struct timer {
  uint64_t fire_time_sec;
  uint64_t fire_time_usec;
  timer_handler handler;
  void *arg;
  uint32_t arg_len;
  struct timer *next;
  struct timer *prev;
};


#endif
