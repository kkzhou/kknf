#include "socket_info.h"

#define RECV_BUF_MIN (16*1024)

struct socket_info* create_socket_info(int socket, enum socket_type type, 
                                       enum socket_state state, uint32_t max_packet)
{
  struct socket_info *info = (struct socket_info*)malloc(sizeof (struct socket_info));
  info->socket = socket;
  info->state = state;
  info->type = type;
  info->send_buf = info->recv_buf = 0;
  info->recv_buf_len = RECV_BUF_MIN;
  info->recv_buf_len = max_pakcet > RECV_BUF_MIN ? max_packet : RECV_BUF_MIN;
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

  if (info->packet_len == 0) {
    /* packet length hasn't been read yet*/
    if (info->recv_buf_used >= 4) {
      info->packet_len = ntohl(*((uint32_t*)info->recv_buf));
      if (info->packet_len == 0 || info->packet_len > info->recv_buf_len_max) {
        return -1;
      } else if (info->packet_len > info->recv_buf_len) {
        /* realloc a larger buffer */
        char *old = info->recv_buf;
        info->recv_buf = (char*)malloc(info->recv_buf_len_max);
      } else {
      }
    }
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
      if (info->packet_len == info->recv_buf_used) {
        ret = 0;
      } else {
        ret = 1;
      }
    } else {
      ret = -2;
    }
    
  } else if (ret == 0) {
    ret = -3;
  } else {
    assert(0);
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
  if (info->type == T_TCP_LISTEN) {
    ret = process_accept(info);
    if (ret == 0) {
      /* complete */
    } else if (ret == 1) {
      /* continue */
    } else if (ret == -1) {
      /* data corrupt */
    } else if (ret == -2) {
      /* error */
    } else if (ret == -3) {
      /* */
    } else {
    }
  } else if (info->type == T_TCP_SERVER_LV) {
    if (info->state == S_IN_RECV || info->state & S_IN_PROCESS) {
      process_recv_lv_format(info);
    }
  }
}
