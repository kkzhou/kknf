
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <stdlib.h>

#include "zxb_context.h"

int zxb_context_create_tcp_listener(uint32_t net_ip, uint16_t net_port, int backlog)
{
    struct sockaddr_in addr;
    memset(&addr, sizeof (struct sockaddr_in), 0);
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = net_ip;
    addr.sin_port = net_port;
    
    int sk = socket(PF_INET, SOCK_STREAM, 0);
    if (sk < 0) {
        return -1;
    }
    
    if (bind(sk, (struct sockaddr*)&addr, sizeof (struct sockaddr_in)) != 0) {
        return -2;
    }
    
    int optval;
    
    optval = fcntl(sk, F_GETFD, 0);
    if (optval == -1) {
        return -3;
    }
    if (optval & O_NONBLOCK) {
        return -4;
    }
    optval |= O_NONBLOCK;
    if (fcntl(sk, F_SETFD, optval) == -1) {
        return -5;
    }
    
    if (listen(sk, backlog) != 0) {
        return -6;
    }
    return sk;
}

int zxb_context_connect(uint32_t net_to_ip, uint16_t net_to_port,
                        uint32_t net_my_ip, uint16_t net_my_port)
{
    struct sockaddr_in addr;
    memset(&addr, sizeof (struct sockaddr_in), 0);
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = net_to_ip;
    addr.sin_port = net_to_port;
    
    int sk = socket(PF_INET, SOCK_STREAM, 0);
    if (sk < 0) {
        return -1;
    }
    
    if (net_my_ip != 0) {
        struct sockaddr_in myaddr;
        memset(&myaddr, sizeof (struct sockaddr_in), 0);
        
        myaddr.sin_family = AF_INET;
        myaddr.sin_addr.s_addr = net_to_ip;
        myaddr.sin_port = net_to_port;
        
        if (bind(sk, (struct sockaddr*)&myaddr, sizeof (struct sockaddr_in)) != 0) {
            return -2;
        }
    }
    
    int optval;
    
    optval = fcntl(sk, F_GETFD, 0);
    if (optval == -1) {
        return -3;
    }
    if (optval & O_NONBLOCK) {
        return -4;
    }
    optval |= O_NONBLOCK;
    if (fcntl(sk, F_SETFD, optval) == -1) {
        return -5;
    }
    
    if (connect(sk, (struct sockaddr*)&addr, sizeof (struct sockaddr_in)) != 0) {
        return -6;
    }
    
    return sk;
}

int zxb_context_create_file(char *path)
{
    return 0;
}


struct zxb_context* zxb_context_create(int fd, uint32_t id, enum zxb_context_type type)
{
    assert(fd > 0);
    struct zxb_context *ctx = (struct zxb_context*)malloc(sizeof (struct zxb_context));
    
    memset(ctx, sizeof (struct zxb_context), 0);
    
    ctx->fd = fd;
    ctx->id = id;
    ctx->type = type;
    ctx->read_buf_len = ctx->read_buf_len_min = 1024 * 100;
    ctx->read_buf_len_max = 1024 * 1024 * 2;
    ctx->read_buf = (char*)malloc(ctx->read_buf_len);
    
    return ctx;
}

int zxb_context_add_write_buf(struct zxb_context *ctx, char *data, uint32_t len)
{
    assert(ctx);
    assert(data);
    assert(len > 0);
    
    struct zxb_buf *new_buf = (struct zxb_buf*)malloc(sizeof (struct zxb_buf));
    memset(new_buf, sizeof (struct zxb_buf), 0);
    
    new_buf->buf = data;
    new_buf->len = len;
    
    if (ctx->write_buf_num > 0) {
        
        ctx->write_buf_tail->next = new_buf;
        new_buf->next = 0;
        new_buf->prev = ctx->write_buf_tail;
        ctx->write_buf_tail = new_buf;
        ctx->write_buf_num++;
    } else {
        assert(ctx->write_buf == 0);
        new_buf->next = new_buf->prev = 0;
        ctx->write_buf = ctx->write_buf_tail = new_buf;
        ctx->write_buf_num = 1;
    }
    
    return 0;
}

int zxb_context_set_event_callback(struct zxb_context *ctx, zxb_context_callback cb)
{
    assert(ctx);
    
    ctx->event_callback = cb;
    return 0;
}


int zxb_context_set_error_callback(struct zxb_context *ctx, zxb_context_callback cb)
{
    assert(ctx);
    ctx->error_callback = cb;
    return 0;
}

int zxb_context_destroy(struct zxb_context *ctx)
{
    assert(ctx);
    if (ctx->read_buf)
        free(ctx->read_buf);
    
    if (ctx->arg)
        free(ctx->arg);
    
    while (ctx->write_buf) {
        struct zxb_buf *iter = ctx->write_buf;
        ctx->write_buf = iter->next;
        free(iter->buf);
        free(iter);
    }
    free(ctx);
    return 0;
}
