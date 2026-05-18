#ifndef GPU_ISA_H
#define GPU_ISA_H

#define NUM_REGS 16

#include <stdint.h>

typedef union {
  struct {
    float x, y, z, w;
  };
  struct {
    float r, g, b, a;
  };
  float comp[4];
} VectorRegs;

typedef struct {
  uint32_t PC;
  uint32_t SR;
} SpecRegs;

typedef enum {
  // memory & control flow (0x00 - 0x0f)
  GVG_NOP = 0x00,
  GVG_COPY = 0x01, // copy register to register
  GVG_JUMP = 0x02, // jump
  GVG_JZ = 0x03,   // jump if zero

  // trigonometric (0x10 - 0x1f)
  GVG_SIN = 0x10,    // sine
  GVG_COS = 0x11,    // cosine
  GVG_ATAN2 = 0x12,  // arctan2
  GVG_SINCOS = 0x13, // sin & cos

  // spatial & geometric (0x20 - 0x3f)
  GVG_DOT = 0x20,   // dot product
  GVG_CROSS = 0x21, // cross product
  GVG_NORM = 0x22,  // normalize
  GVG_LEN = 0x23,   // magnitude
  GVG_REFL = 0x24,  // reflect
  GVG_LERP = 0x25,  // linear interpolate

  // math (0x30 - 0x3f)
  GVG_SCALE = 0x30, // multiply
  GVG_SHIFT = 0x31, // add
  GVG_INV = 0x32,   // reciprocal

  // predication (0x40 - 0x4f)
  GVG_STEP = 0x40,   // step function
  GVG_CLAMP = 0x41,  // clamp
  GVG_CMP = 0x42,    // compare: SR = (src1 > src2) ? 1 : 0
  GVG_CMP_EQ = 0x43, // equality: SR = (src1 == src2) ? 1 : 0

  // framebuffer & rendering (0x50 - 0x5f)
  GVG_CLEAR = 0x50,    // clear framebuffer
  GVG_SETPIXEL = 0x51, // write pixel (x, y, color)
  GVG_GETPIXEL = 0x52, // read pixel (x, y)
  GVG_SETDEPTH = 0x53, // write depth
  GVG_GETDEPTH = 0x54, // read depth
} OpCode;

// component rearrangement
typedef enum {
  BROAD_XYZW = 0x00, // no change
  BROAD_X = 0x01,    // broadcast x
  BROAD_Y = 0x02,    // broadcast y
  BROAD_Z = 0x03,    // broadcast z
  BROAD_W = 0x04,    // broadcast w
} Broad;

// write masks for components
#define MASK_X (1 << 0)
#define MASK_Y (1 << 1)
#define MASK_Z (1 << 2)
#define MASK_W (1 << 3)
#define MASK_ALL (MASK_X | MASK_Y | MASK_Z | MASK_W)

// 32-bit instruction format
// For regular ops: addr field holds dest(bits 0-3) | src1(bits 4-7) | src2(bits
// 8-11) For JUMP/JZ: addr field holds target PC address (bits 0-11, allows
// 0-4095)
typedef struct {
  uint32_t opcode : 8;  // bits 0-7
  uint32_t addr : 12;   // bits 8-19 (multipurpose: operands or address)
  uint32_t swizzle : 4; // bits 20-23
  uint32_t w_mask : 4;  // bits 24-27
  uint32_t padding : 4; // bits 28-31
} InstrFields;

// extract operands from addr field for regular ops
#define INSTR_DEST(a) ((a >> 0) & 0xF)
#define INSTR_SRC1(a) ((a >> 4) & 0xF)
#define INSTR_SRC2(a) ((a >> 8) & 0xF)

typedef union {
  uint32_t raw;
  InstrFields fields;
} Instruction;

typedef struct {
  SpecRegs specRegs;
  VectorRegs regs[NUM_REGS];
  uint8_t *framebuffer;
  uint32_t width;
  uint32_t height;
} VirtualGPU;
#endif /* ifndef GPU_ISA_H */
