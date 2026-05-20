#ifndef CPU_MAP_H
#define CPU_MAP_H

#include "../ISA/ISA.h"
#include "ExecUnit.h"
#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>

#define NUM_CORES 24
#define EUS_PER_CORE 8
#define TOTAL_EUS (NUM_CORES * EUS_PER_CORE)

typedef struct Thread_Pool Thread_Pool;

typedef struct {
  uint32_t core_id;
  EU_State eus[EUS_PER_CORE];
  pthread_t thread;
  Thread_Pool *pool;
} Thread_Context;

struct Thread_Pool {
  Thread_Context threads[NUM_CORES];
  VirtualGPU *gpu;
  Instruction *program;
  uint32_t program_size;
  bool running;
};

Thread_Pool *cpu_map_init(VirtualGPU *gpu);

void cpu_map_execute(Thread_Pool *pool, Instruction *program, uint32_t program_size);

void cpu_map_shutdown(Thread_Pool *pool);

#endif // CPU_MAP_H
