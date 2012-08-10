#ifndef __SOCKET_INFO_H__
#define  __SOCKET_INFO_H__
enum socket_state {
  S_IN_CONNECT = 1,
  S_CONNECTED,
  S_IN_SEND,
  S_IN_RECEIVE,
  S_IN_PROCESS,
  S_IDLE
};

struct socketinfo {
  int socket;
  char *recv_buf;
  uint32_t recv_buf_len;
  uint32_t recv_buf_pos;
  uint32_t send_buf_len;
  uint32_t send_buf_pos;
};

#endif
