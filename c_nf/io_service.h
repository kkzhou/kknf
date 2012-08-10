typedef int (*io_callback)(void*);

struct timer_context {
  struct timeval fire_time;
  uint32_t interval;
  io_callback handler;
};


struct socket_context {
  uint32_t timeout;
  int socket;
  enum socket_state state;
  io_callback handler;
};

struct io_service {
  uint32_t max_slot_size;
  
};
