# Copilot Instructions for Geometric Virtual GPU

## Project Purpose

A CPU-parallel rendering framework that emulates a virtual GPU using a custom geometric ISA. Users can leverage all CPU cores for 2D/3D rendering by either:
- Using high-level graphics API functions (wrapping ISA)
- Writing custom ISA assembly programs

The ISA layer (parsing + execution) maps to low-level C functions that execute in parallel across CPU cores, presenting a hardware GPU-like interface on top of CPU compute.

## Architecture

### Three-Layer Design

**Layer 1: ISA Core (ISA/ directory)**
- ISA.h: OpCode enum, instruction format, VirtualGPU struct
- parser.c/h: Assembly text → binary instructions with robust error handling
- execution.c/h: Instruction dispatch and per-operation handlers
- renderer.c/h (future): SDL2 integration, framebuffer, CPU work dispatch

**Layer 2: User-Facing API (api/ directory, future)**
- High-level graphics functions: gvg_draw_triangle(), gvg_rotate_2d(), etc.
- Context management: gvg_create_context(), gvg_execute_program()
- Each function compiles to ISA internally

**Layer 3: Application Layer (main/ directory, future)**
- SDL2 window setup and render loop
- Users call either API functions or load custom ISA assembly
- **16 vector registers (V0–V15)** storing 4-component float values (RGBA or XYZW)
- **16MB VRAM** simulated as linear byte array
- **32-bit instruction format** using bitfield encoding:
  - Bits 0–7: Opcode
  - Bits 8–11: Destination register
  - Bits 12–15: Source 1 register
  - Bits 16–19: Source 2 register
  - Bits 20–23: Swizzle configuration
  - Bits 24–27: Write mask
  - Bits 28–31: Reserved

### Opcode Categories
- **Memory & Control Flow (0x00–0x0F)**: NOP, LOAD, STORE, JUMP, JZ
- **Trigonometric (0x10–0x2F)**: SIN, COS, TAN, ASIN, ACOS, ATAN, ATAN2, SINCOS
- **Geometric/Spatial (0x30–0x4F)**: DOT, CROSS, NORM, LEN, REFL, LERP
- **Math Primitives (0x50–0x6F)**: SCALE (multiply), SHIFT (add), INV (reciprocal)
- **Predication (0x70–0x7F)**: STEP, CLAMP

### Pipeline

1. **Parser (parser.c/h)**: Converts assembly text to 32-bit binary instructions
   - Maps opcode strings (e.g., "GVG_SIN") to enum values
   - Encodes register indices and write masks into bitfield
   - Returns instruction count on success, -1 on error

2. **Execution (execution.c/h)**: Fetches, decodes, and executes instructions
   - `operation_map(VirtualGPU *gpu)`: Main dispatch function
   - Reads instruction from RAM at PC, decodes fields, calls operation handler
   - Individual handlers (e.g., `gvg_sin()`) implement operations
   - Returns status code: 0 = success, 1 = out of bounds, 2 = unknown opcode

## CPU Parallelism Model

Each frame:
1. **Prepare**: Divide work into batches (pixels, vertices, etc.)
2. **Dispatch**: Split batches across available CPU cores (pthread/OpenMP)
3. **Execute**: Each core runs ISA instruction buffer on local/shared VirtualGPU state
4. **Sync**: Barrier synchronization waits all cores to complete
5. **Render**: Merge results into framebuffer
6. **Present**: SDL2 updates screen

VirtualGPU struct is thread-safe; each core can have:
- Independent PC (program counter) if using instruction batches
- Shared V[0-15] registers (synchronized via atomics)
- Local working state as needed

## Key Implementation Patterns

### Instruction Decoding
```c
Instruction instr;
memcpy(&instr.raw, &gpu->RAM[gpu->registers.PC], sizeof(uint32_t));
OpCode op = (OpCode)instr.fields.opcode;
uint8_t dest = instr.fields.dest;
uint8_t src1 = instr.fields.src1;
uint8_t src2 = instr.fields.src2;
```

### Write Mask Application
```c
if (instr.fields.w_mask & MASK_X) gpu->V[dest].x = result_x;
if (instr.fields.w_mask & MASK_Y) gpu->V[dest].y = result_y;
if (instr.fields.w_mask & MASK_Z) gpu->V[dest].z = result_z;
if (instr.fields.w_mask & MASK_W) gpu->V[dest].w = result_w;
```

### Swizzle Application (Component Rearrangement)
- `SWIZZLE_XYZW` (0x00): No change
- `SWIZZLE_XXXX` (0x01): Broadcast X component
- `SWIZZLE_YYYY` (0x02): Broadcast Y component
- `SWIZZLE_ZZZZ` (0x03): Broadcast Z component
- `SWIZZLE_WWWW` (0x04): Broadcast W component

Apply swizzle **before** the operation on input registers, then apply write mask **after** on the result.

## Code Conventions

### File Organization
- `ISA.h`: Type definitions, instruction format, register layout, opcode enums
- `parser.c/h`: Text-to-binary assembly conversion
- `execution.c/h`: Instruction dispatch and operation implementations

### Function Signatures
Operations take `(VirtualGPU *gpu, Instruction instr)` and return `uint8_t` status code.

### Error Codes
- **0**: Success
- **1**: Out of bounds (PC overflow)
- **2**: Unknown opcode
- Other codes: Operation-specific (document in function)

### Naming Conventions
- Opcodes: `GVG_<OPERATION>` (geometric vector GPU prefix)
- Operation handlers: `gvg_<operation>()`
- Write mask constants: `MASK_<COMPONENT>` (MASK_X, MASK_Y, etc.)
- Swizzle enums: `SWIZZLE_<PATTERN>` (SWIZZLE_XYZW, SWIZZLE_XXXX, etc.)

## Important Notes

- **No build system yet**: Compilation is manual. Files are C/C++ headers and C source.
- **Early stage**: Parser only handles 8 opcodes (NOP, SHIFT, SCALE, SINCOS, LEN, LOAD, STORE). Add more as needed to `parse_opcode_str()`.
- **Execution partially implemented**: Most operation functions are declared but empty stubs in `execution.c`.
- **Bug in execution.c**: Line 13 uses `memccpy()` which may not work as intended; should verify this is `memcpy()` with correct parameters.

## When Contributing

1. Respect the 32-bit instruction encoding format—don't break the bitfield layout.
2. Keep operation implementations independent; each should extract its own operands from the instruction.
3. Apply swizzle to inputs, operation logic to values, write mask to outputs—in that order.
4. Add new opcodes to `ISA.h` enum, then to `parse_opcode_str()` in `parser.c`, then implement in `execution.c`.
5. Return appropriate status codes; don't silently fail.

## Future Expansion

- Additional opcodes for matrix operations, texture sampling, or rasterization
- SIMD-style batching for parallel vector operations
- Assembly syntax improvements (labels, constants, comments)
- Debugging symbols and instruction tracing
