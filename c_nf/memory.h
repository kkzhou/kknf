#ifndef __MEMORY_H__
#define __MEMORY_H__

void* mem_create_ctx();
void* mem_get(void *mem_ctx, uint32_t size);
void mem_put(void *mem_ctx, void *data);

#endif
