#include "socket_info.h"
#include "timer.h"

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


void* get_io_service();

int tcp_send_to(void *io_service, char *ipstr, uint32_t host_port, data_handler after_send_handler = 0);
int sync_tcp_send_and_recv(void *io_service, char *ipstr, uint32_t host_port, data_handler after_send_handler = 0);
int tcp_listen_at(void *io_service, char *ipstr, uint32_t host_port, int backlog,
                  data_handler message_handler = 0,
                  data_handler trunk_handler = 0,
                  data_handler is_trunk_callback = 0,
                  data_handler is_message_callback = 0,
                  data_handler error_handler = 0,
                  data_handler after_send_handler = 0);

int udp_send_to(void *io_service, char *ipstr, uint32_t host_port, data_handler after_send_handler = 0);
int sync_udp_send_and_recv(void *io_service, char *ipstr, uint32_t host_port, data_handler after_send_handler = 0);
int udp_listen_at(void *io_service, char *ipstr, uint32_t host_port,
                  data_handler message_handler = 0,
                  data_handler error_handler = 0,
                  data_handler after_send_handler = 0);


int add_timer(char *name, uint64_t fire_time_sec, uint64_t fire_time_usec, timer_handler h);
int cancel_timer(char *name);
