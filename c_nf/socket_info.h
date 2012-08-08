#ifndef __SOCKET_INFO_H__
#define  __SOCKET_INFO_H__
struct socketinfo {
  int socket;
  char *recv_buf;
  uint32_t recv_buf_len;
  uint32_t recv_buf_pos;
  uint32_t send_buf_len;
  uint32_t send_buf_pos;
};
#endif
