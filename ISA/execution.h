#ifndef EXEC_H
#define EXEC_H

#include "../core/ExecUnit.h"
#include "ISA.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

uint8_t operation_map(EU_State *restrict eu, Instruction instr);

#endif // !EXEC_H
