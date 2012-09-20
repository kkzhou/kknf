#ifndef __ZXB_CONTEXT_H__
#define  __ZXB_CONTEXT_H__

#include <inttypes.h>

struct zxb_context;
typedef int(*zxb_context_callback)(struct zxb_context *ctx, void*);

enum zxb_context_type {
    T_FILE = 1,
    T_TCP,
    T_UDP,
    T_TIMER
};

struct zxb_buf {
    uint32_t net_ip;
    uint16_t net_port;
    char *buf;
    uint32_t len;
    struct zxb_buf *prev;
    struct zxb_buf *next;
};

struct zxb_context {
    
    enum zxb_context_type type;
    int fd;                               /* >=0:file descriptor, <0: timer's id  */
    uint32_t id;                          /* Unique id used by upper layer */
    
    zxb_context_callback event_callback;
    zxb_context_callback error_callback;
    
    struct zxb_buf *write_buf;
    struct zxb_buf *write_buf_tail;
    uint32_t write_buf_num;
    uint32_t begin_of_left_bytes;
    
    char *read_buf;
    uint32_t read_buf_len;
    uint32_t read_buf_use;
    uint32_t read_buf_len_min;
    uint32_t read_buf_len_max;
    
    /* for timer */
    uint64_t fire_time_sec;
    uint64_t fire_time_usec;
    void *arg;
    
    struct zxb_context *prev;
    struct zxb_context *next;  
};


/* util functions */
int zxb_context_create_tcp_listener(uint32_t net_ip, uint16_t net_port, int backlog);
int zxb_context_create_udp_listener(uint32_t net_ip, uint16_t net_port);
int zxb_context_connect(uint32_t net_to_ip, uint16_t net_to_port,
                        uint32_t net_my_ip, uint16_t net_my_port);
int zxb_context_create_file(char *path);
uint32_t zxb_context_ipstr_to_addr(char *ipstr);

/* context manipulate functions */
struct zxb_context* zxb_context_create(int fd, uint32_t id, enum zxb_context_type type);
int zxb_context_add_write_buf(struct zxb_context *ctx, char *data, uint32_t len);
int zxb_context_set_event_callback(struct zxb_context *ctx, zxb_context_callback cb);
int zxb_context_set_error_callback(struct zxb_context *ctx, zxb_context_callback cb);
int zxb_context_destroy(struct zxb_context *ctx);

#endif
