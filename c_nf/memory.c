#include <stdint.h>
#include "memory.h"

struct mem_pool {
  void *base;
  uint32_t block_size;
  uint32_t block_used;
  uint32_t block_num;
};

#define RANK_NUM 100
#define POOL_NUM 100
#define BLOCK_SIZE_BASE 1024
#define BLOCK_SIZE_SCALE 1
#define POOL_SIZE (1024 * 1024 * 1024)

struct mem_ctx {
  struct mem_pool pools[RANK_NUM][POOL_NUM];
  uint32_t used_pool[POOL_NUM];
};

struct mem_ctx* mem_ctx_create()
{
  struct mem_ctx *ret = (struct mem_ctx*)malloc(sizeof (struct mem_pool));

  for (int i = 0; i < RANK_NUM; i++) {
    for (int j = 0; j < 1; j++) {
      ret->pools[i][j].block_size = (i + BLOCK_SIZE_SCALE) * BLOCK_SIZE_BASE;
      ret->pools[i][j].block_used = 0;
      ret->pools[i][j].block_num = POOL_SIZE / ret->pools[i][j].block_size;
      ret->pools[i][j].base = malloc(POOL_SIZE);
    }
    ret->used_pool[i] = 1;
  }
  return ret;
}

void* mem_get(struct mem_ctx *ctx, uint32_t size)
{
  assert(ctx);
  assert(size != 0);

  uint32_t rank = size / BLOCK_SIZE_BASE - BLOCK_SIZE_SCALE;
  assert(rank < POOL_NUM && rank >= 0);

  
}
