#include "CPU_map.h"
#include "../ISA/execution.h"
#include <stdlib.h>
#include <string.h>

static void *thread_worker(void *arg) {
  Thread_Context *ctx = (Thread_Context *)arg;
  Instruction *prog = ctx->pool->program;
  uint32_t prog_size = ctx->pool->program_size;

  for (uint32_t eu_idx = 0; eu_idx < EUS_PER_CORE; eu_idx++) {
    EU_State *eu = &ctx->eus[eu_idx];
    eu->specRegs.PC = 0;

    while (eu->specRegs.PC < prog_size) {
      operation_map(eu, prog[eu->specRegs.PC]);
    }
  }

  return NULL;
}

Thread_Pool *cpu_map_init(VirtualGPU *gpu) {
  Thread_Pool *pool = (Thread_Pool *)malloc(sizeof(Thread_Pool));
  if (!pool)
    return NULL;

  pool->gpu = gpu;
  pool->program = NULL;
  pool->program_size = 0;
  pool->running = false;

  uint32_t tile_cols = 16;
  uint32_t tile_rows = 16;
  uint32_t tile_width = gpu->width / tile_cols;
  uint32_t tile_height = gpu->height / tile_rows;

  uint32_t eu_id = 0;

  for (uint32_t core = 0; core < NUM_CORES; core++) {
    pool->threads[core].core_id = core;

    for (uint32_t eu_idx = 0; eu_idx < EUS_PER_CORE; eu_idx++) {
      EU_State *eu = &pool->threads[core].eus[eu_idx];

      eu->eu_id = eu_id++;
      eu->total_eus = TOTAL_EUS;
      eu->gpu = gpu;

      for (int i = 0; i < NUM_REGS; i++) {
        eu->regs[i].x = 0.0f;
        eu->regs[i].y = 0.0f;
        eu->regs[i].z = 0.0f;
        eu->regs[i].w = 0.0f;
      }

      eu->specRegs.PC = 0;
      eu->specRegs.SR = 0;

      uint32_t tile_idx = eu->eu_id;
      uint32_t tile_row = tile_idx / tile_cols;
      uint32_t tile_col = tile_idx % tile_cols;

      eu->tile_x = tile_col * tile_width;
      eu->tile_y = tile_row * tile_height;
      eu->tile_width = tile_width;
      eu->tile_height = tile_height;
    }

    pool->threads[core].thread = 0;
  }

  return pool;
}

void cpu_map_execute(Thread_Pool *pool, Instruction *program,
                     uint32_t program_size) {
  pool->program = program;
  pool->program_size = program_size;
  pool->running = true;

  memset(pool->gpu->framebuffer, 0, pool->gpu->width * pool->gpu->height * 4);

  for (uint32_t core = 0; core < NUM_CORES; core++) {
    pool->threads[core].pool = pool;
    pthread_create(&pool->threads[core].thread, NULL, thread_worker,
                   &pool->threads[core]);
  }

  for (uint32_t core = 0; core < NUM_CORES; core++) {
    pthread_join(pool->threads[core].thread, NULL);
  }

  pool->running = false;
}

void cpu_map_shutdown(Thread_Pool *pool) {
  if (!pool)
    return;
  free(pool);
}
