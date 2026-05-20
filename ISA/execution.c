#include "execution.h"
#include "ISA.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// memory & control flow
static inline uint8_t gvg_nop(EU_State *restrict eu, Instruction instr) {
  return 0;
}

static inline uint8_t gvg_copy(EU_State *restrict eu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int src = INSTR_SRC1(instr.fields.addr);
  eu->regs[dest] = eu->regs[src];
  return 0;
}

static inline uint8_t gvg_jump(EU_State *restrict eu, Instruction instr) {
  eu->specRegs.PC = instr.fields.addr;
  return 0;
}

static inline uint8_t gvg_jz(EU_State *restrict eu, Instruction instr) {
  if (eu->specRegs.SR == 0)
    eu->specRegs.PC = instr.fields.addr;
  else
    eu->specRegs.PC++;
  return 0;
}

// trigonometric
static inline uint8_t gvg_sin(EU_State *restrict eu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int src = INSTR_SRC1(instr.fields.addr);

  eu->regs[dest].x = sinf(eu->regs[src].x);
  eu->regs[dest].y = sinf(eu->regs[src].y);
  eu->regs[dest].z = sinf(eu->regs[src].z);
  eu->regs[dest].w = sinf(eu->regs[src].w);

  return 0;
}

static inline uint8_t gvg_cos(EU_State *restrict eu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int src = INSTR_SRC1(instr.fields.addr);

  eu->regs[dest].x = cosf(eu->regs[src].x);
  eu->regs[dest].y = cosf(eu->regs[src].y);
  eu->regs[dest].z = cosf(eu->regs[src].z);
  eu->regs[dest].w = cosf(eu->regs[src].w);

  return 0;
}

static inline uint8_t gvg_atan2(EU_State *restrict eu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int src1 = INSTR_SRC1(instr.fields.addr);
  int src2 = INSTR_SRC2(instr.fields.addr);

  eu->regs[dest].x = atan2f(eu->regs[src1].x, eu->regs[src2].x);
  eu->regs[dest].y = atan2f(eu->regs[src1].y, eu->regs[src2].y);
  eu->regs[dest].z = atan2f(eu->regs[src1].z, eu->regs[src2].z);
  eu->regs[dest].w = atan2f(eu->regs[src1].w, eu->regs[src2].w);

  return 0;
}

static inline uint8_t gvg_sincos(EU_State *restrict eu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int src = INSTR_SRC1(instr.fields.addr);

  eu->regs[dest].x = sinf(eu->regs[src].x);
  eu->regs[dest].y = cosf(eu->regs[src].x);
  eu->regs[dest].z = 0.0f;
  eu->regs[dest].w = 0.0f;

  return 0;
}

// spatial & geometric
static inline uint8_t gvg_dot(EU_State *restrict eu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int src1 = INSTR_SRC1(instr.fields.addr);
  int src2 = INSTR_SRC2(instr.fields.addr);

  float dot = eu->regs[src1].x * eu->regs[src2].x +
              eu->regs[src1].y * eu->regs[src2].y +
              eu->regs[src1].z * eu->regs[src2].z;

  eu->regs[dest].x = dot;
  eu->regs[dest].y = 0.0f;
  eu->regs[dest].z = 0.0f;
  eu->regs[dest].w = 0.0f;

  return 0;
}

static inline uint8_t gvg_cross(EU_State *restrict eu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int src1 = INSTR_SRC1(instr.fields.addr);
  int src2 = INSTR_SRC2(instr.fields.addr);

  eu->regs[dest].x = eu->regs[src1].y * eu->regs[src2].z -
                      eu->regs[src1].z * eu->regs[src2].y;
  eu->regs[dest].y = eu->regs[src1].z * eu->regs[src2].x -
                      eu->regs[src1].x * eu->regs[src2].z;
  eu->regs[dest].z = eu->regs[src1].x * eu->regs[src2].y -
                      eu->regs[src1].y * eu->regs[src2].x;
  eu->regs[dest].w = 0.0f;

  return 0;
}

static inline uint8_t gvg_norm(EU_State *restrict eu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int src = INSTR_SRC1(instr.fields.addr);

  float len = sqrtf(eu->regs[src].x * eu->regs[src].x +
                    eu->regs[src].y * eu->regs[src].y +
                    eu->regs[src].z * eu->regs[src].z);

  if (len != 0.0f) {
    eu->regs[dest].x = eu->regs[src].x / len;
    eu->regs[dest].y = eu->regs[src].y / len;
    eu->regs[dest].z = eu->regs[src].z / len;
  }
  eu->regs[dest].w = eu->regs[src].w;

  return 0;
}

static inline uint8_t gvg_len(EU_State *restrict eu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int src = INSTR_SRC1(instr.fields.addr);

  float len = sqrtf(eu->regs[src].x * eu->regs[src].x +
                    eu->regs[src].y * eu->regs[src].y +
                    eu->regs[src].z * eu->regs[src].z);

  eu->regs[dest].x = len;
  eu->regs[dest].y = 0.0f;
  eu->regs[dest].z = 0.0f;
  eu->regs[dest].w = 0.0f;

  eu->specRegs.SR = (len > 0.0f) ? 1 : 0;

  return 0;
}

static inline uint8_t gvg_refl(EU_State *restrict eu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int incident = INSTR_SRC1(instr.fields.addr);
  int normal = INSTR_SRC2(instr.fields.addr);

  float dot = eu->regs[incident].x * eu->regs[normal].x +
              eu->regs[incident].y * eu->regs[normal].y +
              eu->regs[incident].z * eu->regs[normal].z;

  eu->regs[dest].x = eu->regs[incident].x - 2.0f * dot * eu->regs[normal].x;
  eu->regs[dest].y = eu->regs[incident].y - 2.0f * dot * eu->regs[normal].y;
  eu->regs[dest].z = eu->regs[incident].z - 2.0f * dot * eu->regs[normal].z;
  eu->regs[dest].w = eu->regs[incident].w;

  return 0;
}

static inline uint8_t gvg_lerp(EU_State *restrict eu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int src1 = INSTR_SRC1(instr.fields.addr);
  int src2 = INSTR_SRC2(instr.fields.addr);

  float t = eu->regs[src2].x;

  eu->regs[dest].x =
      eu->regs[src1].x + t * (eu->regs[src2].x - eu->regs[src1].x);
  eu->regs[dest].y =
      eu->regs[src1].y + t * (eu->regs[src2].y - eu->regs[src1].y);
  eu->regs[dest].z =
      eu->regs[src1].z + t * (eu->regs[src2].z - eu->regs[src1].z);
  eu->regs[dest].w =
      eu->regs[src1].w + t * (eu->regs[src2].w - eu->regs[src1].w);

  return 0;
}

// math
static inline uint8_t gvg_scale(EU_State *restrict eu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int src1 = INSTR_SRC1(instr.fields.addr);
  int src2 = INSTR_SRC2(instr.fields.addr);

  eu->regs[dest].x = eu->regs[src1].x * eu->regs[src2].x;
  eu->regs[dest].y = eu->regs[src1].y * eu->regs[src2].y;
  eu->regs[dest].z = eu->regs[src1].z * eu->regs[src2].z;
  eu->regs[dest].w = eu->regs[src1].w * eu->regs[src2].w;

  return 0;
}

static inline uint8_t gvg_shift(EU_State *restrict eu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int src1 = INSTR_SRC1(instr.fields.addr);
  int src2 = INSTR_SRC2(instr.fields.addr);

  eu->regs[dest].x = eu->regs[src1].x + eu->regs[src2].x;
  eu->regs[dest].y = eu->regs[src1].y + eu->regs[src2].y;
  eu->regs[dest].z = eu->regs[src1].z + eu->regs[src2].z;
  eu->regs[dest].w = eu->regs[src1].w + eu->regs[src2].w;

  return 0;
}

static inline uint8_t gvg_inv(EU_State *restrict eu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int src = INSTR_SRC1(instr.fields.addr);

  eu->regs[dest].x =
      (eu->regs[src].x != 0.0f) ? 1.0f / eu->regs[src].x : 0.0f;
  eu->regs[dest].y =
      (eu->regs[src].y != 0.0f) ? 1.0f / eu->regs[src].y : 0.0f;
  eu->regs[dest].z =
      (eu->regs[src].z != 0.0f) ? 1.0f / eu->regs[src].z : 0.0f;
  eu->regs[dest].w =
      (eu->regs[src].w != 0.0f) ? 1.0f / eu->regs[src].w : 0.0f;

  return 0;
}

// predication
static inline uint8_t gvg_step(EU_State *restrict eu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int edge = INSTR_SRC1(instr.fields.addr);
  int src = INSTR_SRC2(instr.fields.addr);

  eu->regs[dest].x = (eu->regs[src].x >= eu->regs[edge].x) ? 1.0f : 0.0f;
  eu->regs[dest].y = (eu->regs[src].y >= eu->regs[edge].y) ? 1.0f : 0.0f;
  eu->regs[dest].z = (eu->regs[src].z >= eu->regs[edge].z) ? 1.0f : 0.0f;
  eu->regs[dest].w = (eu->regs[src].w >= eu->regs[edge].w) ? 1.0f : 0.0f;

  return 0;
}

static inline uint8_t gvg_clamp(EU_State *restrict eu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int src = INSTR_SRC1(instr.fields.addr);
  int minv = INSTR_SRC2(instr.fields.addr);

  float min_val = eu->regs[minv].x;
  float max_val = eu->regs[minv].y;

  eu->regs[dest].x = (eu->regs[src].x < min_val)   ? min_val
                      : (eu->regs[src].x > max_val) ? max_val
                                                     : eu->regs[src].x;
  eu->regs[dest].y = (eu->regs[src].y < min_val)   ? min_val
                      : (eu->regs[src].y > max_val) ? max_val
                                                     : eu->regs[src].y;
  eu->regs[dest].z = (eu->regs[src].z < min_val)   ? min_val
                      : (eu->regs[src].z > max_val) ? max_val
                                                     : eu->regs[src].z;
  eu->regs[dest].w = (eu->regs[src].w < min_val)   ? min_val
                      : (eu->regs[src].w > max_val) ? max_val
                                                     : eu->regs[src].w;

  return 0;
}

static inline uint8_t gvg_cmp(EU_State *restrict eu, Instruction instr) {
  int src1 = INSTR_SRC1(instr.fields.addr);
  int src2 = INSTR_SRC2(instr.fields.addr);

  float mag1 = sqrtf(eu->regs[src1].x * eu->regs[src1].x +
                     eu->regs[src1].y * eu->regs[src1].y +
                     eu->regs[src1].z * eu->regs[src1].z);
  float mag2 = sqrtf(eu->regs[src2].x * eu->regs[src2].x +
                     eu->regs[src2].y * eu->regs[src2].y +
                     eu->regs[src2].z * eu->regs[src2].z);

  eu->specRegs.SR = (mag1 > mag2) ? 1 : 0;

  return 0;
}

static inline uint8_t gvg_cmp_eq(EU_State *restrict eu, Instruction instr) {
  int src1 = INSTR_SRC1(instr.fields.addr);
  int src2 = INSTR_SRC2(instr.fields.addr);

  float mag1 = sqrtf(eu->regs[src1].x * eu->regs[src1].x +
                     eu->regs[src1].y * eu->regs[src1].y +
                     eu->regs[src1].z * eu->regs[src1].z);
  float mag2 = sqrtf(eu->regs[src2].x * eu->regs[src2].x +
                     eu->regs[src2].y * eu->regs[src2].y +
                     eu->regs[src2].z * eu->regs[src2].z);

  eu->specRegs.SR = (fabsf(mag1 - mag2) < 0.0001f) ? 1 : 0;

  return 0;
}

// framebuffer & rendering
static inline uint8_t gvg_clear(EU_State *restrict eu, Instruction instr) {
  if (!eu->gpu->framebuffer || eu->gpu->width == 0 || eu->gpu->height == 0)
    return 1;

  uint_fast32_t size = eu->gpu->width * eu->gpu->height * 4;
  memset(eu->gpu->framebuffer, 0, size);
  return 0;
}

static inline uint8_t gvg_setpixel(EU_State *restrict eu,
                                   Instruction instr) {
  uint_fast32_t dest = INSTR_DEST(instr.fields.addr);
  uint_fast32_t src = INSTR_SRC1(instr.fields.addr);

  if (!eu->gpu->framebuffer || dest >= 16 || src >= 16)
    return 1;

  uint_fast32_t x = (uint_fast32_t)eu->regs[dest].x;
  uint_fast32_t y = (uint_fast32_t)eu->regs[dest].y;

  if (x >= eu->gpu->width || y >= eu->gpu->height)
    return 0; // clip silently

  uint_fast32_t offset = (y * eu->gpu->width + x) * 4;
  eu->gpu->framebuffer[offset + 0] = (uint_fast8_t)(eu->regs[src].r * 255.0f);
  eu->gpu->framebuffer[offset + 1] = (uint_fast8_t)(eu->regs[src].g * 255.0f);
  eu->gpu->framebuffer[offset + 2] = (uint_fast8_t)(eu->regs[src].b * 255.0f);
  eu->gpu->framebuffer[offset + 3] = (uint_fast8_t)(eu->regs[src].a * 255.0f);

  return 0;
}

static inline uint8_t gvg_getpixel(EU_State *restrict eu,
                                   Instruction instr) {
  uint_fast32_t dest = INSTR_DEST(instr.fields.addr);
  uint_fast32_t src = INSTR_SRC1(instr.fields.addr);

  if (!eu->gpu->framebuffer || dest >= 16 || src >= 16)
    return 1;

  uint_fast32_t x = (uint_fast32_t)eu->regs[src].x;
  uint_fast32_t y = (uint_fast32_t)eu->regs[src].y;

  if (x >= eu->gpu->width || y >= eu->gpu->height) {
    eu->regs[dest].r = eu->regs[dest].g = eu->regs[dest].b =
        eu->regs[dest].a = 0.0f;
    return 0;
  }

  uint_fast32_t offset = (y * eu->gpu->width + x) * 4;
  eu->regs[dest].r = (float)eu->gpu->framebuffer[offset + 0] / 255.0f;
  eu->regs[dest].g = (float)eu->gpu->framebuffer[offset + 1] / 255.0f;
  eu->regs[dest].b = (float)eu->gpu->framebuffer[offset + 2] / 255.0f;
  eu->regs[dest].a = (float)eu->gpu->framebuffer[offset + 3] / 255.0f;

  return 0;
}

static inline uint8_t gvg_setdepth(EU_State *restrict eu,
                                   Instruction instr) {
  uint_fast32_t dest = INSTR_DEST(instr.fields.addr);
  uint_fast32_t src = INSTR_SRC1(instr.fields.addr);

  if (!eu->gpu->depthbuffer || dest >= 16 || src >= 16)
    return 1;

  uint_fast32_t x = (uint_fast32_t)eu->regs[dest].x;
  uint_fast32_t y = (uint_fast32_t)eu->regs[dest].y;

  if (x >= eu->gpu->width || y >= eu->gpu->height)
    return 0; // clip silently

  uint_fast32_t offset = y * eu->gpu->width + x;
  eu->gpu->depthbuffer[offset] = eu->regs[src].x;

  return 0;
}

static inline uint8_t gvg_getdepth(EU_State *restrict eu,
                                   Instruction instr) {
  uint_fast32_t dest = INSTR_DEST(instr.fields.addr);
  uint_fast32_t src = INSTR_SRC1(instr.fields.addr);

  if (!eu->gpu->depthbuffer || dest >= 16 || src >= 16)
    return 1;

  uint_fast32_t x = (uint_fast32_t)eu->regs[src].x;
  uint_fast32_t y = (uint_fast32_t)eu->regs[src].y;

  if (x >= eu->gpu->width || y >= eu->gpu->height) {
    eu->regs[dest].x = 0.0f;
    return 0;
  }

  uint_fast32_t offset = y * eu->gpu->width + x;
  eu->regs[dest].x = eu->gpu->depthbuffer[offset];
  eu->regs[dest].y = 0.0f;
  eu->regs[dest].z = 0.0f;
  eu->regs[dest].w = 0.0f;

  return 0;
}

uint8_t operation_map(EU_State *restrict eu, Instruction instr) {
  OpCode op = (OpCode)instr.fields.opcode;
  uint8_t status = 0;

  switch (op) {
  case GVG_NOP:
    status = gvg_nop(eu, instr);
    break;
  case GVG_COPY:
    status = gvg_copy(eu, instr);
    break;
  case GVG_JUMP:
    status = gvg_jump(eu, instr);
    break;
  case GVG_JZ:
    status = gvg_jz(eu, instr);
    break;
  case GVG_SIN:
    status = gvg_sin(eu, instr);
    break;
  case GVG_COS:
    status = gvg_cos(eu, instr);
    break;
  case GVG_ATAN2:
    status = gvg_atan2(eu, instr);
    break;
  case GVG_SINCOS:
    status = gvg_sincos(eu, instr);
    break;
  case GVG_DOT:
    status = gvg_dot(eu, instr);
    break;
  case GVG_CROSS:
    status = gvg_cross(eu, instr);
    break;
  case GVG_NORM:
    status = gvg_norm(eu, instr);
    break;
  case GVG_LEN:
    status = gvg_len(eu, instr);
    break;
  case GVG_REFL:
    status = gvg_refl(eu, instr);
    break;
  case GVG_LERP:
    status = gvg_lerp(eu, instr);
    break;
  case GVG_SCALE:
    status = gvg_scale(eu, instr);
    break;
  case GVG_SHIFT:
    status = gvg_shift(eu, instr);
    break;
  case GVG_INV:
    status = gvg_inv(eu, instr);
    break;
  case GVG_STEP:
    status = gvg_step(eu, instr);
    break;
  case GVG_CLAMP:
    status = gvg_clamp(eu, instr);
    break;
  case GVG_CMP:
    status = gvg_cmp(eu, instr);
    break;
  case GVG_CMP_EQ:
    status = gvg_cmp_eq(eu, instr);
    break;
  case GVG_CLEAR:
    status = gvg_clear(eu, instr);
    break;
  case GVG_SETPIXEL:
    status = gvg_setpixel(eu, instr);
    break;
  case GVG_GETPIXEL:
    status = gvg_getpixel(eu, instr);
    break;
  case GVG_SETDEPTH:
    status = gvg_setdepth(eu, instr);
    break;
  case GVG_GETDEPTH:
    status = gvg_getdepth(eu, instr);
    break;
  default:
    printf("runtime error: unknown opcode\n");
    return 2;
  }

  if (op != GVG_JUMP && op != GVG_JZ)
    eu->specRegs.PC++;

  return status;
}
