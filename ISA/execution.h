#ifndef EXEC_H
#define EXEC_H

#include "ISA.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define M_PI_6 M_PI / 6
#define M_PI_3 M_PI / 3

uint8_t operation_map(VirtualGPU *restrict gpu);

void gvg_emulator_cycle(VirtualGPU *gpu);
#endif // !EXEC_H
