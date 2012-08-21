#include "socket_info.h"

#define RECV_BUF_MIN (16*1024)
#define RECV_BUF_MAX (4*1024*1024)

struct io_service {
  /* maintain a sorted list of server socket, used in finding idle socket */
  struct socket_info *server_list_head;
  struct socket_info *server_list_tail;
  /* maintain a map, key is 2bytes from ip address
   * here we use the lowwer 2 bytes of ip address
   * because in a server-side situation the backend
   * servers are in the same subnet such as 192.16.*.*
   * when use 10.*.*.*, we should change the 'hash' method
   */
  struct socket_info *client_map[65535];

  uint64_t idle_timeout;
};

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

static struct socket_info* find_idle_client_socket_info(struct io_service *io, char *ipstr, uint16_t host_port)
{
  
  struct socket_info *map[] = io->client_map;

  assert(map);
  struct in_addr ipaddr;
  if (inet_aton(ipstr, &ipaddr) != 1) {
    return 0;
  }

  struct socket_info *ret = 0;
  uint16_t hash_code = *((uint16_t*)(((char*)&ipaddr.s_addr) + 2));
  if (map[hash_code]) {
    ret = map[hash_code];
    if (map[hash_code]->next) {
      map[hash_code]->next->prev = 0;
      map[hash_code] = map[hash_code]->next;
    } else {
      map[hash_code] = 0;
    }
  } else {
    ret = 0;
  }
  return ret;
}

static void return_client_socket_info(struct io_service *io, struct socket_info *info)
{

  struct socket_info *map[] = io->client_map;
  assert(map);

  uint16_t hash_code = *((uint16_t*)(((char*)&info->net_ip) + 2));
  if (map[hash_code]) {
    info->next = map[hash_code];
    info->prev = 0;
    map[hash_code]->prev = info;
  } else {
    info->next = info->prev = 0;
    map[hash_code] = info;
  }

}

static void mark_server_socket_info(struct io_service *io, strstruct socket_info *info)
{
  assert(io);
  assert(info);

  struct timeval now;
  gettimeofday(&now, 0);
  info->atime_sec = now.tv_sec;
  info->atime_usec = now.tv_usec;

  if (!info->prev) {
    assert(io->server_list_head == info);
  } else {

    if (io->server_list_tail == info) {
      io->server_list_tail = info->prev;
    }
    info->prev->next = info->next;

    info->prev = 0;
    info->next = io->server_list_head;
    io->server_list_head = info;
  }
}

static int destroy_idle_server_socket_info(struct io_service *io)
{
  struct socket_info *iter, *old;
  
  struct timeval now;
  gettimeofday(&now, 0);
  int ret = 0;
  iter = io->server_list_tail;
  while (iter) {
    if ((now.tv_sec - iter->atime_sec) * 1000000 + now.tv_usec - iter->atime_usec < io->idle_timeout) {
      break;
    }
    old = iter;
    destroy_socket_info(iter);
    ret++;
    iter = old->prev;
  }
  if (iter) {
    io->server_list_tail = iter;
  } else {
    io->server_list_head = io->server_list_tail = 0;
  }
  return ret;
}

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

int create_client_socket(char *ip_str, uint32_t host_port)
{

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

void reset_socket_info(struct socket_info *info, int socket = -1)
{
  assert(info);
  if (socket >= 0) {
    close(info->socket);
    info->socket = socket;
  }
  info->recv_buf = (char*)malloc(info->recv_buf_len_min);
  info->recv_buf_len = info->recv_buf_len_min;
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
    info->packet_len = info->is_message_callback(info->socket, info->recv_buf, info->recv_buf_used);
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
    /* if this is a listen socket */
    process_code = process_accept(info);
    if (process_code == 0) {
      /* complete */
    } else if (process_code == 1) {
      /* continue */
      
    } else {
      /* error */
      handle_code = info->error_handler(info->socket, 0 1); /* recreate a listen socket */
      if (handle_code != 0) {
        exit(1);                        /* MUST be replaced by a real exit_function */
      }
    }

  } else if (info->type == T_TCP_SERVER_LV || info->type == T_TCP_CLIENT_LV) {
    /* if this is a data socket(server or client */

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
          /* in process */
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
      if (process_code != 0) {
        /* error */
        handle_code = info->error_handler(info, 1);
      }
    } else {
    }
  } else if (info->type == T_HTTP_CLIENT || info->type == T_HTTP_SERVER) {
    /* if this is an http data socket(server or client) */
    if (info->state == S_IN_RECEIVE || info->state == S_IN_PROCESS) {
      process_code = process_recv_http_format(info);
      if (process_code == 0) {
        /* complete */
        handle_code = info->message_handler(info->socket, info->recv_buf, info->recv_used);
        if (handle_code == 0) {
          /* continue reading */
        } else if (handle_code == 1) {
          /* to send */
        } else {
          /* error */
        }
      } else if (process_code == 2) {
        /* trunk complete */
        /* do nothing */
      } else if (process_code == 1) {
        /* continue */
      } else {
        destroy_socket_info(info);
      }
    } else if (info->state == S_IN_SEND) {
    }

    } else if (process_code == 1) {
      /* trunk complete */
      /* do nothing */
    } else
  } else {
  }
}
