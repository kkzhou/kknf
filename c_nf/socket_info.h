#ifndef __SOCKET_INFO_H__
#define  __SOCKET_INFO_H__

typedef int (*data_handler)(int, void*, uint32_t);

enum socket_type {
  T_TCP_LISTEN = 1,
  T_TCP_SERVER_LV,
  T_TCP_CLIENT_LV,
  T_TCP_SERVER_HTTP,
  T_TCP_CLIENT_HTTP,
  T_UDP
};

void* get_io_service();

int tcp_send_to(void *io_service, char *ipstr, uint32_t host_port, data_handler after_send_handler = 0);

int tcp_listen_at(void *io_service, char *ipstr, uint32_t host_port, int backlog,
                  data_handler message_handler = 0,
                  data_handler trunk_handler = 0,
                  data_handler is_trunk_callback = 0,
                  data_handler is_message_callback = 0,
                  data_handler error_handler = 0,
                  data_handler after_send_handler = 0);

int udp_send_to(void *io_service, char *ipstr, uint32_t host_port, data_handler after_send_handler = 0);

int udp_listen_at(void *io_service, char *ipstr, uint32_t host_port,
                  data_handler message_handler = 0,
                  data_handler error_handler = 0,
                  data_handler after_send_handler = 0);

#endif
