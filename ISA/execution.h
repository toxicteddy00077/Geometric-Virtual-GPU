#ifndef EXEC_H
#define EXEC_H

#include "../core/ExecUnit.h"
#include "ISA.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define M_PI_6 M_PI / 6
#define M_PI_3 M_PI / 3

uint8_t operation_map(EU_State *restrict eu, Instruction instr);

#endif // !EXEC_H
