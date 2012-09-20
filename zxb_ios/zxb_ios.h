#ifndef __ZXB_IOS_H__
#define  __ZXB_IOS_H__

#include "zxb_context.h"

struct zxb_ios {
    /* The vecotro of zxb_context indexed by fd
     * this is for socket and file*/
    struct zxb_contex *ctx_fd_vector;
    uint32_t ctx_fd_vector_size;
    
    /* The sorted list of zxb_context of timers*/
    struct zxb_context *timer_ctx_slist;
    
    /* The zxb_contex vector index by id which is defined
     * by upper layer. The straight forward usage is to index
     * the channels to the backend servers, and each channel
     * has many connections to one backend*/
    struct zxb_contex *ctx_id_vector;
    uint32_t ctx_id_vector_size;
    
    /* timeout to destroy idle fd */
    uint32_t fd_idle_timeout;
};

struct zxb_ios* zxb_ios_get();
int zxb_ios_tcp_listen_at(struct zxb_ios *ios, uint32_t net_ip, uint16_t net_port);
int zxb_ios_udp_listen_at(struct zxb_ios *ios, uint32_t net_ip, uint16_t net_port);
int zxb_ios_write(struct zxb_ios *ios, int fd, char *data, uint32_t len);
int zxb_ios_udp_write(struct zxb_ios *ios, int fd, char *data, uint32_t len,
                      uint32_t net_ip, uint16_t net_port);
int zxb_ios_set_recv_callback(struct zxb_ios *ios, int fd, zxb_context_callback cb);
int zxb_ios_get_backend(struct zxb_ios *ios, uint32_t channel);
uint32_t zxb_ios_add_timer(struct zxb_ios *ios, uint64_t sec_to_fire, uint64_t usec_to_fire, void *arg);
int zxb_ios_cancel_timer(struct zxb_ios *ios, uint32_t id);
int zxb_ios_destroy_fd(struct zxb_ios *ios, uint32_t fd);

#endif
