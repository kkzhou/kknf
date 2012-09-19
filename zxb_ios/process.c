#include "process.h"

int process_recv_other_format(struct socket_info *info)
{
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
  assert(info->state == S_IN_RECEIVE || info->state == S_IN_RECEIVE_AND_PROCESS);
  assert(info->type == T_TCP_SERVER_LV || info->type == T_TCP_CLIENT_LV);

  char *buf;
  uint32_t buf_len;
  uint32_t packet_len;

  if (!info->packet_len) {
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
   1: partially sent, continue
   2: eagain, continue
   3: connect ok
   -1: error happened
   -2: closed by peer
 */
int process_tcp_send(struct socket_info *info)
{
  assert(info);
  assert(info->state == S_IN_PROCESS || info->state == S_IN_SEND || info->state == S_IN_CONNECT);
  assert(info->type == T_TCP_SERVER_LV || info->type == T_TCP_CLIENT_LV);

  if (info->state ==  S_IN_CONNECT) {
    info->state == S_IN_RECEIVE;
    return 3;
  }

  char *buf;
  uint32_t buf_len;

  struct iovec *vecs = (struct iovec*)malloc(sizeof (struct iovec) * info->send_buf_num);
  struct send_buf *iter = info->send_buf;

  for (uint32_t i = 0; i < info->send_buf_num; i++) {

    vecs[i].iov_base = iter->buf + iter->buf_sent;
    vecs[i].iov_len = iter->buf_len - iter->buf_sent;
    iter = iter->next;
  }

  /* begin to send */
  int cnt = 0;
  cnt = writev(info->socket, vecs, info->send_buf_num);

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

    struct send_buf* tmp = info->send_buf;
    uint32_t send_buf_num = info->send_buf_num;
    uint32_t left = cnt;

    while (info->send_buf) {
      tmp = info->send_buf;
      if (left >= tmp->buf_len - tmp->buf_sent) {
        /*  */
        info->send_buf = tmp->next;
        info->send_buf_num--;
        free(tmp->buf);
        free(tmp);
        tmp = info->send_buf;
        left -= tmp->buf_len - tmp->buf_sent;
        ret = 0;
      } else {
        tmp->buf_sent += left;
        left = 0;
        ret = 1;
      }
    } /* while */
  }

  return ret;
}

int process_recv_http_format(struct socket_info *info)
{
}

int process_recv_udp(struct socket_info *info)
{
}
/*
  return value:
  >=0: new socket
  -1: error cann't handle
  
 */
int process_accept(struct socket_info *info)
{
  assert(info->state == S_IN_ACCEPT);
  
  struct sockaddr_in new_addr;
  socklen_t new_addr_len;
  memset(&new_addr, sizeof (struct sockaddr_in), 0);

  int new_socket = accept(into->socket, (struct sockaddr*)&new_addr, &new_addr_len);

  if (new_socket == -1) {
    if (errno == EINTR) {
    } else if (errno == EWOULDBLOCK) {
    } else if (errno == EMFILE) {
    } else if (errno == ENFILE) {
    } else if (errno == ENOMEM) {
    } else {
      return -1;
    }
  }

  return new_socket;

}
/*
  return value:
  -1: continue epoll
  >0: new socket created by accept()

 */
int process_socket_event(struct socket_info *info)
{
  assert(info);
  
  int ret = 0;
  int process_code, handle_code;

  if (info->type == T_TCP_LISTEN) {
    /* if this is a listen socket */
    process_code = process_accept(info);
    if (process_code >= 0) {
      /* complete */
      return process_code;
    } else {
      /* error */
      handle_code = info->error_handler(info->socket, 0 1); /* recreate a listen socket */
      if (handle_code != 0) {
        exit(1);                        /* MUST be replaced by a real exit_function */
      }
    }

  } else if (info->type == T_TCP_SERVER_LV || info->type == T_TCP_CLIENT_LV) {
    /* if this is a data socket(server or client */

    if (info->state == S_IN_RECEIVE || info->state == S_IN_RECEIVE_AND_PROCESS) {
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
    } else if (info->state == S_IN_CONNECT) {
      /* connecting */
      assert(0);
    }
  } else if (info->type == T_HTTP_CLIENT || info->type == T_HTTP_SERVER) {
   
  } else {
  }
  return -1;
}

int process_connect(struct socket_info *info)
{
  assert(info);
  assert(info->state == S_IN_CONNECT);
  change_socket_state(S_IN_SEND);
  return 0;
}

int process_error(struct socket_info *info)
{
  assert(info);
  destroy_socket_info(info);
  return 0;
}

int process_timer_event(struct timer *timer)
{
  assert(timer);
  int ret = 1;
  if (timer->timer_handler) {
    ret = timer->timer_handler(timer->arg, timer->arglen);
  }

  return ret;
}

