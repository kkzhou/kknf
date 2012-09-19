#include "io_service.h"

static struct socket_info* get_idle_client_socket_info(struct io_service *io, char *ipstr, uint16_t host_port)
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

  struct timeval now;
  gettimeofday(&now, 0);
  info->atime_sec = now.tv_sec;
  info->atime_usec = now.tv_usec;

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

  iter = old;
  while (iter) {
    old = iter;
    iter = iter->next;
    destroy_socket_info(old);
  }
  return ret;
}


int tcp_send_to(void *io_service, char *ipstr, uint32_t host_port, data_handler after_send_handler = 0)
{
  assert(io_serviice);
  assert(ipastr);

  struct io_service *io = (struct io_service*)io_service;
  
}
