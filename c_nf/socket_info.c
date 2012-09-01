#include "socket_info.h"

#define RECV_BUF_MIN (16*1024)
#define RECV_BUF_MAX (4*1024*1024)

int default_message_handler(int socket, void *buf, uint32_t buflen)
{
  free(buf);
  return 0;
}

int default_trunk_handler(int socket, void *buf, uint32_t buflen)
{
  return 0;
}

int default_error_handler(int socket, void *buf, uint32_t buflen)
{
  return 0;
}

int default_after_send_handler(int socket, void *buf, uint32_t buflen)
{
  return 0;
}

int default_is_trunk_callback(int socket, void *buf, uint32_t buflen)
{
  return 0;
}

int default_is_message_callback(int socket, void *buf, uint32_t buflen)
{
  return 0;
}

struct socket_info* create_socket_info(int socket, enum socket_type type, 
                                       enum socket_state state,
                                       uint32_t packet_min, uint32_t packet_max,
                                       data_handler message_handler = 0,
                                       data_handler trunk_handler = 0,
                                       data_handler is_trunk_callback = 0,
                                       data_handler is_message_callback = 0,
                                       data_handler error_handler = 0,
                                       data_handler after_send_handler = 0)
{

  assert(socket > 0);
  assert(packet_min < packet_max);

  struct socket_info *info = (struct socket_info*)malloc(sizeof (struct socket_info));
  info->socket = socket;
  info->state = state;
  info->type = type;
  info->send_buf = info->recv_buf = 0;
  info->recv_buf_len = info->recv_buf_len_min = packet_min;
  info->recv_buf_len_min = packet_max;
  info->recv_buf = (char*)malloc(info->recv_buf_len);
 
  if (message_handler) {
    info->message_handler = message_handler;
  } else {
    info->message_handler = default_message_handler;
  }

  if (trunk_handler) {
    info->trunk_handler = trunk_handler;
  } else {
    info->trunk_handler = default_trunk_handler;
  }

  if (error_handler) {
    info->error_handler = error_handler;
  } else {
    info->error_handler = default_error_handler;
  }

  if (after_send_handler) {
    info->after_send_handler = after_send_handler;
  } else {
    info->after_send_handler = default_after_send_handler;
  }

  if (is_message_callback) {
    info->is_message_callback = is_message_callback;
  } else {
    info->is_message_callback = default_is_message_callback;
  }

  if (is_trunk_callback) {
    info->is_trunk_callback = is_trunk_callback;
  } else {
    info->is_trunk_callback = default_is_trunk_callback;
  }  
    
  return info;
}

int create_tcp_client_socket(char *ip_str, uint32_t host_port)
{
  assert(ip_str);
  struct sockaddr_in addr;
  memset(&addr, sizeof (struct sockaddr_in), 0);
  int socket = socket(PF_INET, SOCK_STREAM, 0);
  if (socket < 0) {
    return -1;
  }
  if (inet_aton(ip_str, &addr.sin_addr) != 0) {
    return -2;
  }
  addr.sin_port = htons(host_port);
  addr.sin_family = AF_INET;
  set_nonblock(socket);
  int ret = connect(socket, (struct sockaddr*)(&addr), sizeof(struct sockaddr));
  if (ret == -1 && errno != EINPROCESS)
    return -3;
  }

  return socket;

}

int create_tcp_listen_socket(char *ip_str, uint32_t host_port, int backlog)
{
  assert(ip_str);
  struct sockaddr_in addr;
  memset(&addr, sizeof (struct sockaddr_in), 0);
  int socket = socket(PF_INET, SOCK_STREAM, 0);
  if (socket < 0) {
    return -1;
  }
  if (inet_aton(ip_str, &addr.sin_addr) != 0) {
    return -2;
  }
  addr.sin_port = htons(host_port);
  addr.sin_family = AF_INET;
  if (bind(socket, (struct sockaddr*)(&addr), sizeof(struct sockaddr)) != 0) {
    return -3;
  }
  if (listen(socket, backlog) != 0) {
    return -4;
  }
  return socket;
}

int create_udp_socket(char *ip_str, uint16_t host_port)
{
  assert(ip_str);
  struct sockaddr_in addr;
  memset(&addr, sizeof (struct sockaddr_in), 0);
  int socket = socket(PF_INET, SOCK_DATAGRM, 0);
  if (socket < 0) {
    return -1;
  }
  if (inet_aton(ip_str, &addr.sin_addr) != 0) {
    return -2;
  }
  addr.sin_port = htons(host_port);
  addr.sin_family = AF_INET;
  if (bind(socket, (struct sockaddr*)(&addr), sizeof(struct sockaddr)) != 0) {
    return -3;
  }

  return socket;
}

void destroy_socket_info(struct socket_info *info)
{
  assert(info);
  close(info->socket);
  if (info->recv_buf) {
    free(info->recv_buf);
  } 
  if (info->send_buf) {
    free(info->send_buf);
  }
  free(info);
    
}

void reset_socket_info(struct socket_info *info, int socket = -1)
{
  assert(info);
  if (socket >= 0) {
    close(info->socket);
    info->socket = socket;
    info->send_buf = 0;
    info->send_buf_len = 0;
    info->recv_buf_len = info->recv_buf_len_min;
    info->recv_buf_used = 0;
  } else {
    info->recv_buf = (char*)malloc(info->recv_buf_len_min);
    info->recv_buf_len = info->recv_buf_len_min;
  }
  
}

data_handler set_handler(struct socket_info *info, data_handler h,
                         int which/*1:trunk_handler,
                                    2:is_trunk_callback,
                                    3:message_handler,
                                    4:is_message_callback,
                                    5:after_send_handler,
                                    6:error_handler*/)
{
  data_handler old = 0;
  switch (which) {
    case 1:
      old = info->trunk_handler;
      info->trunk_handler = h;
      break;
    case 2:
      old = info->is_trunk_callback;
      info->is_trunk_callback = h;
      break;
    case 3:
      old = info->message_handler;
      info->message_handler = h;
      break;
    case 4:
      old = info->is_message_callback;
      info->is_message_callback = h;
      break;
    case 5:
      old = info->after_send_handler;
      info->after_send_handler = h;
      break;
    default:
      assert(0);
      break;
  }

  return old;
}

int change_socket_state(struct socket_info *info, enum socket_state state)
{
  assert(info);
  info->state = state;
  return 0;
}

int set_nonblock(int socket)
{
  int optval;

  optval = fcntl(socket, F_GETFD, 0);
  if (optval == -1) {
    return -1;
  }
  if (optval & O_NONBLOCK) {
    return -2;
  }
  optval |= O_NONBLOCK;
  if (fcntl(socket, F_SETFD, optval) == -1) {
    return -3;
  }
  return 0;
}

int set_reuse(int socket)
{
  int optval = 1;
  size_t optlen = sizeof (int);
  if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &optval, optlen) < 0) {
    return -1;
  }
  return 0;
}
