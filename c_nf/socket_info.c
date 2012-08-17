#include "socket_info.h"

#define RECV_BUF_MIN (16*1024)
#define RECV_BUF_MAX (4*1024*1024)

struct socket_info* create_socket_info(int socket, enum socket_type type, 
                                       enum socket_state state,
                                       uint32_t packet_min, uint32_t packet_max)
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

  return info;
}

int create_listen_socket(char *ip_str, uint32_t host_port, int backlog)
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

void reset_socket_info(struct socket_info *info)
{
  assert(info);
  info->recv_buf = (char*)malloc(info->recv_buf_len_min);
  info->recv_buf_len = info->recv_buf_len_min;
}

data_handler set_handler(struct socket_info *info, data_handler h,
                         int which/*1:trunk_handler,
                                    2:is_trunk_callback,
                                    3:message_handler,
                                    4:is_message_callback,
                                    5:after_send_handler*/)
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

/**
   return value:
   0: complete
   1: continue
   -1: packet length invalid
   -2: error happened
   -3: closed by peer
 */
int process_recv_lv_format(struct socket_info *info)
{
  assert(info);
  assert(info->state & S_IN_PROCESS);
  assert(info->state == S_IN_RECV);
  assert(info->type == T_TCP_SERVER_LV || info->type == T_TCP_CLIENT_LV);

  char *buf;
  uint32_t buf_len;
  uint32_t packet_len;

  if (info->packet_len) {
    info->packet_len = info->is_message_callback(info->recv_buf, info->recv_buf_used);
  }

  if (info->packet_len > info->recv_buf_len_max) {
    return -1;
  } else if (info->packet_len > info->recv_buf_len) {
    /* realloc a larger buffer */
    char *old = info->recv_buf;
    info->recv_buf_len = info->recv_buf_len_max;
    info->recv_buf = (char*)malloc(info->recv_buf_len);
    memcpy(info->recv_buf, old, info->recv_buf_used);
    free(old);
  } else {
  }
   
  buf = info->recv_buf + info->recv_buf_used;
  buf_len = info->packet_len - (info->recv_buf_len - info->recv_buf_used + 1);

  /* begin to recv */
  int cnt = 0;
  uint32_t cur = 0;
  uint32_t bytes_read = 0;

  while ((cnt = recv(info->socket, buf + cur, buf_len - bytes_read, 0)) > 0) {
    cur += cnt;
    bytes_read -= cnt;
    if (cur == buf_len) {
      break;
    }
  }
  info->recv_buf_used += cur;

  int ret = 0;
  if (cnt < 0) {
    if (errno == EAGAIN) {
      if (info->packet_len != 0 && info->packet_len == info->recv_buf_used) {
        ret = 0;
      } else {
        ret = 1;
      }
    } else {
      ret = -2;
    }
    
  } else if (cnt == 0) {
    ret = -3;
  } else {
    
  }

  return ret;
}

/**
   return value:
   0: complete
   1: continue
   -1: error happened
   -2: closed by peer
 */
int process_send(struct socket_info *info)
{
  assert(info);
  assert(info->state & S_IN_PROCESS);
  assert(info->state == S_IN_SEND);
  assert(info->type == T_TCP_SERVER_LV || info->type == T_TCP_CLIENT_LV);

  char *buf;
  uint32_t buf_len;

  buf = info->send_buf + info->send_buf_sent;
  buf_len = info->send_buf_len - info->send_buf_sent;

  /* begin to send */
  int cnt = 0;
  uint32_t cur = 0;
  uint32_t bytes_sent = 0;

  while ((cnt = send(info->socket, buf + cur, buf_len - bytes_sent, 0)) > 0) {
    cur += cnt;
    bytes_sent -= cnt;
    if (cur == buf_len) {
      break;
    }
  }
  info->send_buf_sent += cur;

  int ret = 1;
  if (cnt < 0) {
    if (errno == EAGAIN) {
      ret = 1;
    } else {
      ret = -1;
    }
  } else if (cnt == 0) {
    ret = -2;
  } else {
    ret = 0;
  }

  return ret;
}

int process_socket(struct socket_info *info)
{
  assert(info);
  
  int ret = 0;
  int process_code, handle_code;

  if (info->type == T_TCP_LISTEN) {
    process_code = process_accept(info);

  } else if (info->type == T_TCP_SERVER_LV || info->type == T_TCP_CLIENT_LV) {
    if (info->state == S_IN_RECEIVE || info->state & S_IN_PROCESS) {
      process_code = process_recv_lv_format(info);
      if (process_code == 0) {
        /* complete */
        handle_code = info->message_handler(info->recv_buf, info->recv_buf_used);
        reset_socket_info(info);
        if (handle_code == 0) {
          /* continue reading */
        } else if (handle_code == -1) {
          /* close socket */
          destroy_socket_info(info);
          ret = -1;
        } else if (handle_code == 1) {
          /* to send */
          set_socket_state(info, S_IN_SEND);
        } else if (handle_code == 2) {
          /* in process set idle */
          set_socket_state(info, S_IN_PROCESS);
        } else {
          assert(0);
        }

      } else if (process_code == 1) {
        /* continue */
      } else if (process_code < 0) {
        /* error */
        destroy_socket_info(info);
      } else {
        assert(0);
      }
    } else if (info->state == S_IN_SEND) {
      process_code = process_send(info);
    } else {
    }
  } else if (info->type == T_HTTP) {
  } else {
  }
}
