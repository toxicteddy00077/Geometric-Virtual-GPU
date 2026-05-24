#include "ISA/memory.h"
#include "ISA/parser.h"
#include "core/CPU_map.h"
#include "core/engine.h"
#include <math.h>
#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdio.h>

#define WIDTH 800
#define HEIGHT 600
#define MAX_INSTRUCTIONS 1024

int main(void) {
  VirtualGPU *gpu = gvg_alloc_gpu(WIDTH, HEIGHT);
  if (!gpu) { fprintf(stderr, "failed to alloc gpu\n"); return 1; }

  uint32_t binary[MAX_INSTRUCTIONS];
  int instr_count = assmbler("tests/cube.gvg", binary, MAX_INSTRUCTIONS);
  if (instr_count < 0) { fprintf(stderr, "failed to parse tests/cube.gvg\n"); return 1; }
  Instruction *program = (Instruction *)binary;

  Thread_Pool *pool = cpu_map_init(gpu);
  if (!pool) { fprintf(stderr, "failed to init thread pool\n"); return 1; }

  for (int c = 0; c < NUM_CORES; c++) {
    for (int e = 0; e < EUS_PER_CORE; e++) {
      EU_State *eu = &pool->threads[c].eus[e];
      eu->regs[8].x = eu->regs[8].y = eu->regs[8].z = eu->regs[8].w = 1.0f;
      eu->regs[9].x = eu->regs[9].y = eu->regs[9].z = eu->regs[9].w = 0.002f;
      eu->regs[11].x = eu->regs[11].y = eu->regs[11].z = eu->regs[11].w = -1.0f;
    }
  }

  Engine *eng = engine_init(WIDTH, HEIGHT, "Geometric Virtual GPU");
  if (!eng) { fprintf(stderr, "failed to init engine\n"); return 1; }

  const uint64_t frame_ms = 1000 / 60;
  bool running = true;

  while (running) {
    uint64_t frame_start = SDL_GetTicks();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT)
        running = false;
    }

    static float theta = 0.0f;
    static const float cube_verts[8][3] = {
      {-1, -1, -1}, {+1, -1, -1}, {+1, +1, -1}, {-1, +1, -1},
      {-1, -1, +1}, {+1, -1, +1}, {+1, +1, +1}, {-1, +1, +1}
    };
    float scale = 150.0f, cx = 400.0f, cy = 300.0f;
    float tilt    = atan2f(1.0f, sqrtf(2.0f));
    float total_y = theta + (float)M_PI / 4.0f;
    float cy_a = cosf(total_y), sy_a = sinf(total_y);
    float cx_a = cosf(tilt),    sx_a = sinf(tilt);

    for (int i = 0; i < 8; i++) {
      float x = cube_verts[i][0], y = cube_verts[i][1], z = cube_verts[i][2];
      float rx1 = x * cy_a - z * sy_a;
      float ry1 = y;
      float rz1 = x * sy_a + z * cy_a;
      float rx2 = rx1;
      float ry2 = ry1 * cx_a - rz1 * sx_a;
      float px = cx + rx2 * scale;
      float py = cy - ry2 * scale;
      for (int c = 0; c < NUM_CORES; c++)
        for (int e = 0; e < EUS_PER_CORE; e++) {
          pool->threads[c].eus[e].regs[i].x = px;
          pool->threads[c].eus[e].regs[i].y = py;
          pool->threads[c].eus[e].regs[i].z = 0.0f;
          pool->threads[c].eus[e].regs[i].w = 0.0f;
          pool->threads[c].eus[e].specRegs.PC = 0;
        }
    }
    theta += 0.02f;

    cpu_map_execute(pool, program, (uint32_t)instr_count);
    engine_render_frame(eng, gpu);

    uint64_t elapsed = SDL_GetTicks() - frame_start;
    if (elapsed < frame_ms)
      SDL_Delay((uint32_t)(frame_ms - elapsed));
  }

  engine_shutdown(eng);
  cpu_map_shutdown(pool);
  gvg_free_gpu(gpu);
  return 0;
}
