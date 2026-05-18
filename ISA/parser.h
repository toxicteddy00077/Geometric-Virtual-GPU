#ifndef PARSER_H
#define PARSER_H

#include "ISA.h"
#include <stdint.h>

int assmbler(const char *file, uint32_t *binary, int max_instr);
#endif // !PARSER_H
