#include "parser.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"

#define MAX_LABELS 256

typedef struct {
  char name[64];
  int addr;
} Label;

Label labels[MAX_LABELS];
int label_count = 0;

int RSOLV_LABL(const char *name) {
  for (int i = 0; i < label_count; i++) {
    if (strcmp(labels[i].name, name) == 0)
      return labels[i].addr;
  }
  return -1;
}

void ADD_LABL(const char *name, int addr) {
  if (label_count < MAX_LABELS) {
    strncpy(labels[label_count].name, name, 63);
    labels[label_count].addr = addr;
    label_count++;
  }
}

uint8_t parse_opcode_str(const char *str) {
  if (strcmp(str, "GVG_NOP") == 0)
    return GVG_NOP;
  if (strcmp(str, "GVG_COPY") == 0)
    return GVG_COPY;
  if (strcmp(str, "GVG_SHIFT") == 0)
    return GVG_SHIFT;
  if (strcmp(str, "GVG_SCALE") == 0)
    return GVG_SCALE;
  if (strcmp(str, "GVG_SINCOS") == 0)
    return GVG_SINCOS;
  if (strcmp(str, "GVG_LEN") == 0)
    return GVG_LEN;
  if (strcmp(str, "GVG_JUMP") == 0)
    return GVG_JUMP;
  if (strcmp(str, "GVG_JZ") == 0)
    return GVG_JZ;
  if (strcmp(str, "GVG_CLEAR") == 0)
    return GVG_CLEAR;
  if (strcmp(str, "GVG_SETPIXEL") == 0)
    return GVG_SETPIXEL;
  if (strcmp(str, "GVG_GETPIXEL") == 0)
    return GVG_GETPIXEL;
  if (strcmp(str, "GVG_SIN") == 0)
    return GVG_SIN;
  if (strcmp(str, "GVG_COS") == 0)
    return GVG_COS;
  if (strcmp(str, "GVG_DOT") == 0)
    return GVG_DOT;
  if (strcmp(str, "GVG_CROSS") == 0)
    return GVG_CROSS;
  if (strcmp(str, "GVG_NORM") == 0)
    return GVG_NORM;
  if (strcmp(str, "GVG_ATAN2") == 0)
    return GVG_ATAN2;

  return 0xFF;
}

int assmbler(const char *filename, uint32_t *binary, int max_instr) {
  FILE *file = FILE_CHECK(filename,"r");
  if(!file) return -1;
  label_count = 0;
  
  // collect labels
  char line[256];
  int instr_count = 0;
  while (fgets(line, sizeof(line), file)) {
    if (line[0] == '\n' || line[0] == '\r' || line[0] == '#')
      continue;

    char *colon = strchr(line, ':');
    if (colon) {
      char label_name[64];
      int len = colon - line;
      strncpy(label_name, line, len);
      label_name[len] = '\0';
      ADD_LABL(label_name, instr_count);
      continue;
    }

    instr_count++;
  }

  rewind(file);
  instr_count = 0;

  // parse instructions and resolve jumps
  while (fgets(line, sizeof(line), file) && instr_count < max_instr) {
    if (line[0] == '\n' || line[0] == '\r' || line[0] == '#')
      continue;

    // skip labels
    if (strchr(line, ':'))
      continue;

    char op_str[32];
    unsigned int dest = 0, src1 = 0, src2 = 0, mask = 15;
    char target[64] = {0};

    int tokens = sscanf(line, "%s %u %u %u %u", op_str, &dest, &src1, &src2, &mask);
    if (tokens < 1) continue;

    uint8_t opcode = parse_opcode_str(op_str);
    if (opcode == 0xFF) {
      printf("error: unknown instruction '%s'\n", op_str);
      fclose(file);
      return -1;
    }

    Instruction instr;
    instr.raw = 0;
    instr.fields.opcode = opcode;
    instr.fields.w_mask = mask;

    // check if this is JUMP or JZ
    if (opcode == GVG_JUMP || opcode == GVG_JZ) {
      if (tokens < 2) {
        printf("error: %s requires target\n", op_str);
        fclose(file);
        return -1;
      }
      
      // second argument is label or address
      char target_str[64];
      sscanf(line, "%s %s", op_str, target_str);
      
      int target_addr = RSOLV_LABL(target_str);
      if (target_addr == -1) {
        target_addr = atoi(target_str);
        if (target_addr == 0 && strcmp(target_str, "0") != 0) {
          printf("error: unknown label '%s'\n", target_str);
          fclose(file);
          return -1;
        }
      }
      
      instr.fields.addr = target_addr & 0xFFF;
    } else {
      instr.fields.addr = (dest & 0xF) | ((src1 & 0xF) << 4) | ((src2 & 0xF) << 8);
    }

    binary[instr_count] = instr.raw;
    instr_count++;
  }

  fclose(file);
  return instr_count;
}
