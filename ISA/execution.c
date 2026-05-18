#include "execution.h"
#include "ISA.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// forward declarations for static inline operations
static inline uint8_t gvg_nop(VirtualGPU * restrict gpu, Instruction instr);
static inline uint8_t gvg_copy(VirtualGPU * restrict gpu, Instruction instr);
static inline uint8_t gvg_jump(VirtualGPU * restrict gpu, Instruction instr);
static inline uint8_t gvg_jz(VirtualGPU * restrict gpu, Instruction instr);
static inline uint8_t gvg_sin(VirtualGPU * restrict gpu, Instruction instr);
static inline uint8_t gvg_cos(VirtualGPU * restrict gpu, Instruction instr);
static inline uint8_t gvg_atan2(VirtualGPU * restrict gpu, Instruction instr);
static inline uint8_t gvg_sincos(VirtualGPU * restrict gpu, Instruction instr);
static inline uint8_t gvg_dot(VirtualGPU * restrict gpu, Instruction instr);
static inline uint8_t gvg_cross(VirtualGPU * restrict gpu, Instruction instr);
static inline uint8_t gvg_norm(VirtualGPU * restrict gpu, Instruction instr);
static inline uint8_t gvg_len(VirtualGPU * restrict gpu, Instruction instr);
static inline uint8_t gvg_refl(VirtualGPU * restrict gpu, Instruction instr);
static inline uint8_t gvg_lerp(VirtualGPU * restrict gpu, Instruction instr);
static inline uint8_t gvg_scale(VirtualGPU * restrict gpu, Instruction instr);
static inline uint8_t gvg_shift(VirtualGPU * restrict gpu, Instruction instr);
static inline uint8_t gvg_inv(VirtualGPU * restrict gpu, Instruction instr);
static inline uint8_t gvg_step(VirtualGPU * restrict gpu, Instruction instr);
static inline uint8_t gvg_clamp(VirtualGPU * restrict gpu, Instruction instr);
static inline uint8_t gvg_cmp(VirtualGPU * restrict gpu, Instruction instr);
static inline uint8_t gvg_cmp_eq(VirtualGPU * restrict gpu, Instruction instr);
static inline uint8_t gvg_clear(VirtualGPU * restrict gpu, Instruction instr);
static inline uint8_t gvg_setpixel(VirtualGPU * restrict gpu, Instruction instr);
static inline uint8_t gvg_getpixel(VirtualGPU * restrict gpu, Instruction instr);

uint8_t operation_map(VirtualGPU * restrict gpu) {
  Instruction instr;
  instr.raw = 0;

  OpCode op = (OpCode)instr.fields.opcode;
  uint8_t status = 0;

  switch (op) {
  case GVG_NOP:
    status = gvg_nop(gpu, instr);
    break;
  case GVG_COPY:
    status = gvg_copy(gpu, instr);
    break;
  case GVG_JUMP:
    status = gvg_jump(gpu, instr);
    break;
  case GVG_JZ:
    status = gvg_jz(gpu, instr);
    break;
  case GVG_SIN:
    status = gvg_sin(gpu, instr);
    break;
  case GVG_COS:
    status = gvg_cos(gpu, instr);
    break;
  case GVG_ATAN2:
    status = gvg_atan2(gpu, instr);
    break;
  case GVG_SINCOS:
    status = gvg_sincos(gpu, instr);
    break;
  case GVG_DOT:
    status = gvg_dot(gpu, instr);
    break;
  case GVG_CROSS:
    status = gvg_cross(gpu, instr);
    break;
  case GVG_NORM:
    status = gvg_norm(gpu, instr);
    break;
  case GVG_LEN:
    status = gvg_len(gpu, instr);
    break;
  case GVG_REFL:
    status = gvg_refl(gpu, instr);
    break;
  case GVG_LERP:
    status = gvg_lerp(gpu, instr);
    break;
  case GVG_SCALE:
    status = gvg_scale(gpu, instr);
    break;
  case GVG_SHIFT:
    status = gvg_shift(gpu, instr);
    break;
  case GVG_INV:
    status = gvg_inv(gpu, instr);
    break;
  case GVG_STEP:
    status = gvg_step(gpu, instr);
    break;
  case GVG_CLAMP:
    status = gvg_clamp(gpu, instr);
    break;
  case GVG_CMP:
    status = gvg_cmp(gpu, instr);
    break;
  case GVG_CMP_EQ:
    status = gvg_cmp_eq(gpu, instr);
    break;
  case GVG_CLEAR:
    status = gvg_clear(gpu, instr);
    break;
  case GVG_SETPIXEL:
    status = gvg_setpixel(gpu, instr);
    break;
  case GVG_GETPIXEL:
    status = gvg_getpixel(gpu, instr);
    break;
  default:
    printf("runtime error: unknown opcode\n");
    return 2;
  }

  if (op != GVG_JUMP && op != GVG_JZ)
    gpu->specRegs.PC++;

  return status;
}

// memory & control flow
static inline uint8_t gvg_nop(VirtualGPU * restrict gpu, Instruction instr) { return 0; }

static inline uint8_t gvg_copy(VirtualGPU * restrict gpu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int src = INSTR_SRC1(instr.fields.addr);
  gpu->regs[dest] = gpu->regs[src];
  return 0;
}

static inline uint8_t gvg_jump(VirtualGPU * restrict gpu, Instruction instr) {
  gpu->specRegs.PC = instr.fields.addr;
  return 0;
}

static inline uint8_t gvg_jz(VirtualGPU * restrict gpu, Instruction instr) {
  if (gpu->specRegs.SR == 0)
    gpu->specRegs.PC = instr.fields.addr;
  else
    gpu->specRegs.PC++;
  return 0;
}

// trigonometric
static inline uint8_t gvg_sin(VirtualGPU * restrict gpu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int src = INSTR_SRC1(instr.fields.addr);

  gpu->regs[dest].x = sinf(gpu->regs[src].x);
  gpu->regs[dest].y = sinf(gpu->regs[src].y);
  gpu->regs[dest].z = sinf(gpu->regs[src].z);
  gpu->regs[dest].w = sinf(gpu->regs[src].w);

  return 0;
}

static inline uint8_t gvg_cos(VirtualGPU * restrict gpu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int src = INSTR_SRC1(instr.fields.addr);

  gpu->regs[dest].x = cosf(gpu->regs[src].x);
  gpu->regs[dest].y = cosf(gpu->regs[src].y);
  gpu->regs[dest].z = cosf(gpu->regs[src].z);
  gpu->regs[dest].w = cosf(gpu->regs[src].w);

  return 0;
}

static inline uint8_t gvg_atan2(VirtualGPU * restrict gpu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int src1 = INSTR_SRC1(instr.fields.addr);
  int src2 = INSTR_SRC2(instr.fields.addr);

  gpu->regs[dest].x = atan2f(gpu->regs[src1].x, gpu->regs[src2].x);
  gpu->regs[dest].y = atan2f(gpu->regs[src1].y, gpu->regs[src2].y);
  gpu->regs[dest].z = atan2f(gpu->regs[src1].z, gpu->regs[src2].z);
  gpu->regs[dest].w = atan2f(gpu->regs[src1].w, gpu->regs[src2].w);

  return 0;
}

static inline uint8_t gvg_sincos(VirtualGPU * restrict gpu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int src = INSTR_SRC1(instr.fields.addr);

  gpu->regs[dest].x = sinf(gpu->regs[src].x);
  gpu->regs[dest].y = cosf(gpu->regs[src].x);
  gpu->regs[dest].z = 0.0f;
  gpu->regs[dest].w = 0.0f;

  return 0;
}

// spatial & geometric
static inline uint8_t gvg_dot(VirtualGPU * restrict gpu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int src1 = INSTR_SRC1(instr.fields.addr);
  int src2 = INSTR_SRC2(instr.fields.addr);

  float dot = gpu->regs[src1].x * gpu->regs[src2].x +
              gpu->regs[src1].y * gpu->regs[src2].y +
              gpu->regs[src1].z * gpu->regs[src2].z;

  gpu->regs[dest].x = dot;
  gpu->regs[dest].y = 0.0f;
  gpu->regs[dest].z = 0.0f;
  gpu->regs[dest].w = 0.0f;

  return 0;
}

static inline uint8_t gvg_cross(VirtualGPU * restrict gpu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int src1 = INSTR_SRC1(instr.fields.addr);
  int src2 = INSTR_SRC2(instr.fields.addr);

  gpu->regs[dest].x = gpu->regs[src1].y * gpu->regs[src2].z -
                      gpu->regs[src1].z * gpu->regs[src2].y;
  gpu->regs[dest].y = gpu->regs[src1].z * gpu->regs[src2].x -
                      gpu->regs[src1].x * gpu->regs[src2].z;
  gpu->regs[dest].z = gpu->regs[src1].x * gpu->regs[src2].y -
                      gpu->regs[src1].y * gpu->regs[src2].x;
  gpu->regs[dest].w = 0.0f;

  return 0;
}

static inline uint8_t gvg_norm(VirtualGPU * restrict gpu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int src = INSTR_SRC1(instr.fields.addr);

  float len = sqrtf(gpu->regs[src].x * gpu->regs[src].x +
                    gpu->regs[src].y * gpu->regs[src].y +
                    gpu->regs[src].z * gpu->regs[src].z);

  if (len != 0.0f) {
    gpu->regs[dest].x = gpu->regs[src].x / len;
    gpu->regs[dest].y = gpu->regs[src].y / len;
    gpu->regs[dest].z = gpu->regs[src].z / len;
  }
  gpu->regs[dest].w = gpu->regs[src].w;

  return 0;
}

static inline uint8_t gvg_len(VirtualGPU * restrict gpu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int src = INSTR_SRC1(instr.fields.addr);

  float len = sqrtf(gpu->regs[src].x * gpu->regs[src].x +
                    gpu->regs[src].y * gpu->regs[src].y +
                    gpu->regs[src].z * gpu->regs[src].z);

  gpu->regs[dest].x = len;
  gpu->regs[dest].y = 0.0f;
  gpu->regs[dest].z = 0.0f;
  gpu->regs[dest].w = 0.0f;

  gpu->specRegs.SR = (len > 0.0f) ? 1 : 0;

  return 0;
}

static inline uint8_t gvg_refl(VirtualGPU * restrict gpu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int incident = INSTR_SRC1(instr.fields.addr);
  int normal = INSTR_SRC2(instr.fields.addr);

  float dot = gpu->regs[incident].x * gpu->regs[normal].x +
              gpu->regs[incident].y * gpu->regs[normal].y +
              gpu->regs[incident].z * gpu->regs[normal].z;

  gpu->regs[dest].x = gpu->regs[incident].x - 2.0f * dot * gpu->regs[normal].x;
  gpu->regs[dest].y = gpu->regs[incident].y - 2.0f * dot * gpu->regs[normal].y;
  gpu->regs[dest].z = gpu->regs[incident].z - 2.0f * dot * gpu->regs[normal].z;
  gpu->regs[dest].w = gpu->regs[incident].w;

  return 0;
}

static inline uint8_t gvg_lerp(VirtualGPU * restrict gpu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int src1 = INSTR_SRC1(instr.fields.addr);
  int src2 = INSTR_SRC2(instr.fields.addr);

  float t = gpu->regs[src2].x;

  gpu->regs[dest].x =
      gpu->regs[src1].x + t * (gpu->regs[src2].x - gpu->regs[src1].x);
  gpu->regs[dest].y =
      gpu->regs[src1].y + t * (gpu->regs[src2].y - gpu->regs[src1].y);
  gpu->regs[dest].z =
      gpu->regs[src1].z + t * (gpu->regs[src2].z - gpu->regs[src1].z);
  gpu->regs[dest].w =
      gpu->regs[src1].w + t * (gpu->regs[src2].w - gpu->regs[src1].w);

  return 0;
}

// math
static inline uint8_t gvg_scale(VirtualGPU * restrict gpu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int src1 = INSTR_SRC1(instr.fields.addr);
  int src2 = INSTR_SRC2(instr.fields.addr);

  gpu->regs[dest].x = gpu->regs[src1].x * gpu->regs[src2].x;
  gpu->regs[dest].y = gpu->regs[src1].y * gpu->regs[src2].y;
  gpu->regs[dest].z = gpu->regs[src1].z * gpu->regs[src2].z;
  gpu->regs[dest].w = gpu->regs[src1].w * gpu->regs[src2].w;

  return 0;
}

static inline uint8_t gvg_shift(VirtualGPU * restrict gpu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int src1 = INSTR_SRC1(instr.fields.addr);
  int src2 = INSTR_SRC2(instr.fields.addr);

  gpu->regs[dest].x = gpu->regs[src1].x + gpu->regs[src2].x;
  gpu->regs[dest].y = gpu->regs[src1].y + gpu->regs[src2].y;
  gpu->regs[dest].z = gpu->regs[src1].z + gpu->regs[src2].z;
  gpu->regs[dest].w = gpu->regs[src1].w + gpu->regs[src2].w;

  return 0;
}

static inline uint8_t gvg_inv(VirtualGPU * restrict gpu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int src = INSTR_SRC1(instr.fields.addr);

  gpu->regs[dest].x =
      (gpu->regs[src].x != 0.0f) ? 1.0f / gpu->regs[src].x : 0.0f;
  gpu->regs[dest].y =
      (gpu->regs[src].y != 0.0f) ? 1.0f / gpu->regs[src].y : 0.0f;
  gpu->regs[dest].z =
      (gpu->regs[src].z != 0.0f) ? 1.0f / gpu->regs[src].z : 0.0f;
  gpu->regs[dest].w =
      (gpu->regs[src].w != 0.0f) ? 1.0f / gpu->regs[src].w : 0.0f;

  return 0;
}

// predication
static inline uint8_t gvg_step(VirtualGPU * restrict gpu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int edge = INSTR_SRC1(instr.fields.addr);
  int src = INSTR_SRC2(instr.fields.addr);

  gpu->regs[dest].x = (gpu->regs[src].x >= gpu->regs[edge].x) ? 1.0f : 0.0f;
  gpu->regs[dest].y = (gpu->regs[src].y >= gpu->regs[edge].y) ? 1.0f : 0.0f;
  gpu->regs[dest].z = (gpu->regs[src].z >= gpu->regs[edge].z) ? 1.0f : 0.0f;
  gpu->regs[dest].w = (gpu->regs[src].w >= gpu->regs[edge].w) ? 1.0f : 0.0f;

  return 0;
}

static inline uint8_t gvg_clamp(VirtualGPU * restrict gpu, Instruction instr) {
  int dest = INSTR_DEST(instr.fields.addr);
  int src = INSTR_SRC1(instr.fields.addr);
  int minv = INSTR_SRC2(instr.fields.addr);

  float min_val = gpu->regs[minv].x;
  float max_val = gpu->regs[minv].y;

  gpu->regs[dest].x = (gpu->regs[src].x < min_val)   ? min_val
                      : (gpu->regs[src].x > max_val) ? max_val
                                                     : gpu->regs[src].x;
  gpu->regs[dest].y = (gpu->regs[src].y < min_val)   ? min_val
                      : (gpu->regs[src].y > max_val) ? max_val
                                                     : gpu->regs[src].y;
  gpu->regs[dest].z = (gpu->regs[src].z < min_val)   ? min_val
                      : (gpu->regs[src].z > max_val) ? max_val
                                                     : gpu->regs[src].z;
  gpu->regs[dest].w = (gpu->regs[src].w < min_val)   ? min_val
                      : (gpu->regs[src].w > max_val) ? max_val
                                                     : gpu->regs[src].w;

  return 0;
}

static inline uint8_t gvg_cmp(VirtualGPU * restrict gpu, Instruction instr) {
  int src1 = INSTR_SRC1(instr.fields.addr);
  int src2 = INSTR_SRC2(instr.fields.addr);

  float mag1 = sqrtf(gpu->regs[src1].x * gpu->regs[src1].x +
                     gpu->regs[src1].y * gpu->regs[src1].y +
                     gpu->regs[src1].z * gpu->regs[src1].z);
  float mag2 = sqrtf(gpu->regs[src2].x * gpu->regs[src2].x +
                     gpu->regs[src2].y * gpu->regs[src2].y +
                     gpu->regs[src2].z * gpu->regs[src2].z);

  gpu->specRegs.SR = (mag1 > mag2) ? 1 : 0;

  return 0;
}

static inline uint8_t gvg_cmp_eq(VirtualGPU * restrict gpu, Instruction instr) {
  int src1 = INSTR_SRC1(instr.fields.addr);
  int src2 = INSTR_SRC2(instr.fields.addr);

  float mag1 = sqrtf(gpu->regs[src1].x * gpu->regs[src1].x +
                     gpu->regs[src1].y * gpu->regs[src1].y +
                     gpu->regs[src1].z * gpu->regs[src1].z);
  float mag2 = sqrtf(gpu->regs[src2].x * gpu->regs[src2].x +
                     gpu->regs[src2].y * gpu->regs[src2].y +
                     gpu->regs[src2].z * gpu->regs[src2].z);

  gpu->specRegs.SR = (fabsf(mag1 - mag2) < 0.0001f) ? 1 : 0;

  return 0;
}

// framebuffer & rendering
static inline uint8_t gvg_clear(VirtualGPU * restrict gpu, Instruction instr) {
  if (!gpu->framebuffer || gpu->width == 0 || gpu->height == 0)
    return 1;
  
  uint_fast32_t size = gpu->width * gpu->height * 4;
  memset(gpu->framebuffer, 0, size);
  return 0;
}

static inline uint8_t gvg_setpixel(VirtualGPU * restrict gpu, Instruction instr) {
  uint_fast32_t dest = INSTR_DEST(instr.fields.addr);
  uint_fast32_t src = INSTR_SRC1(instr.fields.addr);
  
  if (!gpu->framebuffer || dest >= 16 || src >= 16)
    return 1;
  
  uint_fast32_t x = (uint_fast32_t)gpu->regs[dest].x;
  uint_fast32_t y = (uint_fast32_t)gpu->regs[dest].y;
  
  if (x >= gpu->width || y >= gpu->height)
    return 0;  // clip silently
  
  uint_fast32_t offset = (y * gpu->width + x) * 4;
  gpu->framebuffer[offset + 0] = (uint_fast8_t)(gpu->regs[src].r * 255.0f);
  gpu->framebuffer[offset + 1] = (uint_fast8_t)(gpu->regs[src].g * 255.0f);
  gpu->framebuffer[offset + 2] = (uint_fast8_t)(gpu->regs[src].b * 255.0f);
  gpu->framebuffer[offset + 3] = (uint_fast8_t)(gpu->regs[src].a * 255.0f);
  
  return 0;
}

static inline uint8_t gvg_getpixel(VirtualGPU * restrict gpu, Instruction instr) {
  uint_fast32_t dest = INSTR_DEST(instr.fields.addr);
  uint_fast32_t src = INSTR_SRC1(instr.fields.addr);
  
  if (!gpu->framebuffer || dest >= 16 || src >= 16)
    return 1;
  
  uint_fast32_t x = (uint_fast32_t)gpu->regs[src].x;
  uint_fast32_t y = (uint_fast32_t)gpu->regs[src].y;
  
  if (x >= gpu->width || y >= gpu->height) {
    gpu->regs[dest].r = gpu->regs[dest].g = gpu->regs[dest].b = gpu->regs[dest].a = 0.0f;
    return 0;
  }
  
  uint_fast32_t offset = (y * gpu->width + x) * 4;
  gpu->regs[dest].r = (float)gpu->framebuffer[offset + 0] / 255.0f;
  gpu->regs[dest].g = (float)gpu->framebuffer[offset + 1] / 255.0f;
  gpu->regs[dest].b = (float)gpu->framebuffer[offset + 2] / 255.0f;
  gpu->regs[dest].a = (float)gpu->framebuffer[offset + 3] / 255.0f;
  
  return 0;
}

static inline uint8_t gvg_setdepth(VirtualGPU * restrict gpu, Instruction instr) { return 0; }

static inline uint8_t gvg_getdepth(VirtualGPU * restrict gpu, Instruction instr) { return 0; }

void gvg_emulator_cycle(VirtualGPU *gpu) {}
