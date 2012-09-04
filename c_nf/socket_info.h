#ifndef __SOCKET_INFO_H__
#define  __SOCKET_INFO_H__

typedef int (*data_handler)(int, void*, uint32_t);

struct send_buf {
  struct sockaddr_in to_addr;
  char *buf;
  uint32_t buf_len;
  uint32_t buf_sent;
  struct send_buf *next;
  struct send_buf *prev;

};

struct socket_info {
  int socket;
  uint32_t net_ip;
  uint16_t net_port;

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

  struct send_buf *send_buf;
  uint32_t send_buf_num;

  struct socket_info *prev;
  struct socket_info *next;
};


enum socket_state {
  S_NOT_INIT = 1,
  S_INIT,
  S_IN_CONNECT,
  S_CONNECTED,
  S_IN_ACCEPT,
  S_IN_SEND,
  S_IN_RECEIVE,
  S_IN_PROCESS,
  S_IN_RECVEIVE_AND_PROCESS
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
struct socket_info* create_socket_info(int socket, enum socket_type type, 
                                       enum socket_state state,
                                       uint32_t packet_min, uint32_t packet_max,
                                       data_handler message_handler = 0,
                                       data_handler trunk_handler = 0,
                                       data_handler is_trunk_callback = 0,
                                       data_handler is_message_callback = 0,
                                       data_handler error_handler = 0,
                                       data_handler after_send_handler = 0);

int create_udp_socket(char *ip_str, uint16_t host_port);
int create_tcp_client_socket(char *ip_str, uint32_t host_port);
int create_tcp_listen_socket(char *ip_str, uint32_t host_port, int backlog);
int set_nonblock(int socket);
int change_socket_state(struct socket_info *info, enum socket_state state);
int set_reuse(int socket);
int destroy_socket_info(struct socket_info *info);
void reset_socket_info(struct socket_info *info, int socket = -1);
data_handler set_handler(struct socket_info *info, data_handler h,
                         int which/*1:trunk_handler,
                                    2:is_trunk_callback,
                                    3:message_handler,
                                    4:is_message_callback,
                                    5:after_send_handler,
                                    6:error_handler*/);

#endif
