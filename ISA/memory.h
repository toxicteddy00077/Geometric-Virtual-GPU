#ifndef GPU_MEMORY_H
#define GPU_MEMORY_H

#include "ISA.h"
#include <stdlib.h>
#include <string.h>

VirtualGPU *gvg_alloc_gpu(uint32_t width, uint32_t height);

void gvg_free_gpu(VirtualGPU *gpu);

#endif // !GPU_MEMORY_H
