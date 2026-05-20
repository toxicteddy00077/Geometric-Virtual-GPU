#ifndef GPU_ISA_H
#define GPU_ISA_H

#include <stdint.h>

typedef enum {
  // control flow (0x00 - 0x0f)
  GVG_NOP    = 0x00,
  GVG_COPY   = 0x01,
  GVG_JUMP   = 0x02,
  GVG_JZ     = 0x03,

  // trigonometric (0x10 - 0x1f)
  GVG_SIN    = 0x10,
  GVG_COS    = 0x11,
  GVG_ATAN2  = 0x12,
  GVG_SINCOS = 0x13,

  // geometric (0x20 - 0x2f)
  GVG_DOT    = 0x20,
  GVG_CROSS  = 0x21,
  GVG_NORM   = 0x22,
  GVG_LEN    = 0x23,
  GVG_REFL   = 0x24,
  GVG_LERP   = 0x25,

  // math (0x30 - 0x3f)
  GVG_SCALE  = 0x30,
  GVG_SHIFT  = 0x31,
  GVG_INV    = 0x32,

  // predication (0x40 - 0x4f)
  GVG_STEP   = 0x40,
  GVG_CLAMP  = 0x41,
  GVG_CMP    = 0x42,  // SR = (|src1| > |src2|) ? 1 : 0
  GVG_CMP_EQ = 0x43,  // SR = (|src1| == |src2|) ? 1 : 0

  // framebuffer (0x50 - 0x5f)
  GVG_CLEAR    = 0x50,
  GVG_SETPIXEL = 0x51,
  GVG_GETPIXEL = 0x52,
  GVG_SETDEPTH = 0x53,
  GVG_GETDEPTH = 0x54,

  // eu control (0x60 - 0x6f)
  GVG_EU_TSTART = 0x60,  // dest = eu_id / total_eus (broadcast)
  GVG_EU_TEND   = 0x61,  // dest = (eu_id+1) / total_eus (broadcast)
} OpCode;

typedef enum {
  BROAD_XYZW = 0x00, // no change
  BROAD_X = 0x01,    // broadcast x
  BROAD_Y = 0x02,    // broadcast y
  BROAD_Z = 0x03,    // broadcast z
  BROAD_W = 0x04,    // broadcast w
} Broad;

#define MASK_X (1 << 0)
#define MASK_Y (1 << 1)
#define MASK_Z (1 << 2)
#define MASK_W (1 << 3)
#define MASK_ALL (MASK_X | MASK_Y | MASK_Z | MASK_W)

// 32-bit: [opcode:8][addr:12][swizzle:4][w_mask:4][pad:4]
// addr for regular ops: dest(0-3) | src1(4-7) | src2(8-11)
// addr for JUMP/JZ: target PC (0-4095)
typedef struct {
  uint32_t opcode : 8;  // bits 0-7
  uint32_t addr : 12;   // bits 8-19 (multipurpose: operands or address)
  uint32_t swizzle : 4; // bits 20-23
  uint32_t w_mask : 4;  // bits 24-27
  uint32_t padding : 4; // bits 28-31
} InstrFields;

#define INSTR_DEST(a) ((a >> 0) & 0xF)
#define INSTR_SRC1(a) ((a >> 4) & 0xF)
#define INSTR_SRC2(a) ((a >> 8) & 0xF)

typedef union {
  uint32_t raw;
  InstrFields fields;
} Instruction;

typedef struct {
  uint8_t *framebuffer;
  float *depthbuffer;
  uint32_t width;
  uint32_t height;
} VirtualGPU;
#endif /* ifndef GPU_ISA_H */
