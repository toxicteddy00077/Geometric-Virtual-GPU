#ifndef ENG_H
#define ENG_H

#include "../ISA/ISA.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  uint32_t width;
  uint32_t height;
} Engine;

Engine *engine_init(uint32_t width, uint32_t height, const char *title);

void engine_render_frame(Engine *eng, VirtualGPU *gpu);

void engine_shutdown(Engine *eng);

#endif // !ENG_H
