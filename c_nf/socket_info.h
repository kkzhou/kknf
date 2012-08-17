#ifndef __SOCKET_INFO_H__
#define  __SOCKET_INFO_H__

typedef int (*data_handler)(void*, uint32_t);


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

  uint23_t packet_len;

  char *recv_buf;
  uint32_t recv_buf_len;
  uint32_t recv_buf_used;

  uint32_t recv_buf_len_min;
  uint32_t recv_buf_len_max;


  enum socket_state state;
  enum socket_type type;

  data_handler trunk_handler;
  data_handler message_handler;
  data_handler after_send_handler;
  data_handler is_trunk_callback;
  data_handler is_message_callback;
};

#endif
