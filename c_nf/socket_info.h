#ifndef __SOCKET_INFO_H__
#define  __SOCKET_INFO_H__


enum socket_state {
  S_NOT_INIT = 1,
  S_INIT,

  S_IN_CONNECT,
  S_CONNECTED,

  S_IN_SEND,
  S_IN_RECEIVE,

  S_IDLE,
  /* can used by OR with S_IN_RECV or S_IN_SEND*/
  S_IN_PROCESS = 0x800000
};

enum socket_type {
  T_TCP_LISTEN = 1,
  T_TCP_SERVER,
  T_TCP_CLIENT
};

struct socket_info {
  int socket;
  struct buffer *send_buf;
  struct buffer *recv_buf;
  enum socket_state state;
  enum socket_type type;
};

#endif
