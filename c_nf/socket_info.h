#ifndef __SOCKET_INFO_H__
#define  __SOCKET_INFO_H__

typedef int (*trunk_handler)(void*, uint32_t);
typedef int (*message_handler)(void*, uint32_t);
typedef int (*after_send_handler)(void*, uint32_t);

enum socket_state {
  S_NOT_INIT = 1,
  S_INIT,

  S_IN_CONNECT,
  S_CONNECTED,

  S_IN_SEND,
  S_IN_RECEIVE,

  S_IDLE,
  /* can used by OR */
  S_IN_PROCESS = 0x800000
};

enum socket_type {
  T_TCP_LISTEN = 1,
  T_TCP_SERVER_LV,
  T_TCP_CLIENT_LV,
  T_TCP_SERVER_HTTP,
  T_TCP_CLIENT_HTTP
};

struct socket_info {
  int socket;

  char *send_buf;
  uint32_t send_buf_len;
  uint32_t send_buf_sent;

  uint32_t packet_len;/* the length of the packet we are reading, 0 indicates not knowning yet*/
  char *recv_buf;
  uint32_t recv_buf_len;
  uint32_t recv_buf_used;

  uint32_t recv_buf_len_min;
  uint32_t recv_buf_len_max;


  enum socket_state state;
  enum socket_type type;

  trunk_handler handler1;
  message_handler handler2;
  after_send_handler handler3;
};

#endif
