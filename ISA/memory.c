#include "memory.h"
#include <stdint.h>

VirtualGPU *gvg_alloc_gpu(uint32_t width, uint32_t height) {
  VirtualGPU *gpu = (VirtualGPU *)malloc(sizeof(VirtualGPU));
  if (!gpu)
    return NULL;

  gpu->width = width;
  gpu->height = height;
  gpu->framebuffer = (uint8_t *)malloc(width * height * 4);
  gpu->depthbuffer = (float *)malloc(width * height * sizeof(float));

  if (!gpu->framebuffer || !gpu->depthbuffer) {
    free(gpu->framebuffer);
    free(gpu->depthbuffer);
    free(gpu);
    return NULL;
  }

  memset(gpu->framebuffer, 0, width * height * 4);
  memset(gpu->depthbuffer, 0, width * height * sizeof(float));

  return gpu;
}

void gvg_free_gpu(VirtualGPU *gpu) {
  if (!gpu)
    return;
  free(gpu->framebuffer);
  free(gpu->depthbuffer);
  free(gpu);
}
