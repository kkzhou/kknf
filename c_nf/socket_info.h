#ifndef __SOCKET_INFO_H__
#define  __SOCKET_INFO_H__

typedef int (*data_handler)(int, void*, uint32_t);

struct socket_info {
  int socket;
  uint32_t net_ip;
  uint16_t net_port;

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
  data_handler error_handler;           /* 1: connection brocken */

  uint64_t atime_sec;
  uint64_t atime_usec;

  struct socket_info *prev;
  struct socket_info *next;
};


enum socket_state {
  S_NOT_INIT = 1,
  S_INIT,
  S_IN_CONNECT,
  S_CONNECTED,
  S_IN_SEND,
  S_IN_RECEIVE,
  S_IN_PROCESS = 0x800000               /* it's a bit flag */
};

enum socket_type {
  T_TCP_LISTEN = 1,
  T_TCP_SERVER_LV,
  T_TCP_CLIENT_LV,
  T_TCP_SERVER_HTTP,
  T_TCP_CLIENT_HTTP,
  T_UDP,
  T_TCP_SERVER_OTHER,
  T_TCP_CLIENT_OTHER
};

int set_nonblock(int socket);
int change_socket_state(struct socket_info *info, enum socket_state state);
int set_reuse(int socket);
int destroy_socket_info(struct socket_info *info);
#endif
