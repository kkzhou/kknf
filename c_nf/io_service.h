typedef int (*io_callback)(void*);

#define list_entry(type, node_addr, member) (node_addr) - &((((type)*)0)->(memeber)) 

struct list_node {
  struct list_node next;
  struct list_node prev;
};

struct timer_context {
  struct timeval fire_time;
  uint32_t interval;
  io_callback handler;
};

enum socket_state {
  S_IN_CONNECT = 1,
  S_CONNECTED,
  S_IN_SEND,
  S_IN_RECEIVE,
  S_IN_PROCESS,
  S_IDLE
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
