#include "memory.h"
#include <stdint.h>

VirtualGPU *gvg_alloc_gpu(uint32_t wid, uint32_t bred) {
  VirtualGPU *gpu = (VirtualGPU *)malloc(sizeof(VirtualGPU));
  if (!gpu)
    return NULL;

  memset(gpu, 0, sizeof(VirtualGPU));

  gpu->specRegs.PC = 0;
  gpu->specRegs.SR = 0;

  for (int i = 0; i < NUM_REGS; i++) {
    gpu->regs[i].x = 0.0f;
    gpu->regs[i].y = 0.0f;
    gpu->regs[i].z = 0.0f;
    gpu->regs[i].w = 0.0f;
  }

  gpu->breadth = bred;
  gpu->width = wid;
  uint8_t *framebuffer = (uint8_t *)malloc(sizeof(bred * wid));
  return gpu;
}

void gvg_free_gpu(VirtualGPU *gpu) {
  if (gpu)
    free(gpu);
}
