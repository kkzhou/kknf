
#include "zxb_ios.h"


struct zxb_ios* zxb_ios_get()
{
  struct zxb_ios *ios = (struct zxb_ios*)malloc(sizeof (struct zxb_ios));
  memset(ios, sizeof (struct zxb_ios), 0);

  ios->ctx_fd_vector_size = 100000;
  ios->ctx_fd_vector = (struct zxb_contex*)
      malloc(sizeof (struct zxb_contex) * ios->ctx_fd_vector_size);
  memset(ios->ctx_fd_vector, sizeof (struct zxb_contex) * ios->ctx_fd_vector_size);
  
  ios->ctx_id_vector_size = 1000;
  ios->ctx_id_vector = (uint32_t*)malloc(sizeof (int) * ios->ctx_id_vector_size)
  memset(ios->ctx_id_vector,sizeof (int) * ios->ctx_id_vector_size);

  ios->fd_idle_timeout = 600;           /* destroy when idle for 600s */
  return ios;
}

int zxb_ios_tcp_listen_at(struct zxb_ios *ios, uint32_t net_ip, uint16_t net_port)
{
  int listen_socket = zxb_context_create_tcp_listener(net_ip, net_port, 1024);
  if (listen_socket >= 0) {
    
  }
}

int zxb_ios_udp_listen_at(struct zxb_ios *ios, uint32_t net_ip, uint16_t net_port);
int zxb_ios_write(struct zxb_ios *ios, int fd, char *data, uint32_t len);
int zxb_ios_udp_write(struct zxb_ios *ios, int fd, char *data, uint32_t len,
                      uint32_t net_ip, uint16_t net_port);
int zxb_ios_set_recv_callback(struct zxb_ios *ios, int fd, zxb_contex_callback cb);
int zxb_ios_get_backend(struct zxb_ios *ios, uint32_t channel);
uint32_t zxb_ios_add_timer(struct zxb_ios *ios, uint64_t sec_to_fire, uint64_t usec_to_fire, void *arg);
int zxb_ios_cancel_timer(struct zxb_ios *ios, uint32_t id);
int zxb_ios_destroy_fd(struct zxb_ios *ios, uint32_t fd);
