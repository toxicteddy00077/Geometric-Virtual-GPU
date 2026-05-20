#ifndef EXEC_UNIT_H
#define EXEC_UNIT_H

#include "../ISA/ISA.h"
#include <stdint.h>
#include <stdio.h>

#define NUM_REGS 16

typedef union {
  struct {
    float x, y, z, w;
  };
  struct {
    float r, g, b, a;
  };
  float comp[4];
} VectorRegs;

typedef struct {
  uint32_t PC;
  uint32_t SR;
} SpecRegs;

typedef struct {
  uint32_t eu_id;
  uint32_t total_eus;
  VectorRegs regs[16];
  SpecRegs specRegs;
  VirtualGPU *gpu;
  uint32_t tile_x, tile_y, tile_width, tile_height;
} EU_State;

#endif // !EXEC_UNIT_H
