#include "engine.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>

Engine *engine_init(uint32_t width, uint32_t height, const char *title) {
  Engine *engine = (Engine *)malloc(sizeof(Engine));
  if (!engine) return NULL;

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
    free(engine);
    return NULL;
  }

  engine->width = width;
  engine->height = height;
  engine->window = SDL_CreateWindow(title, width, height, 0);
  if (!engine->window) {
    fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
    free(engine);
    return NULL;
  }

  engine->renderer = SDL_CreateRenderer(engine->window, NULL);
  if (!engine->renderer) {
    fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
    SDL_DestroyWindow(engine->window);
    free(engine);
    return NULL;
  }

  engine->texture = SDL_CreateTexture(engine->renderer, SDL_PIXELFORMAT_ABGR8888,
                                      SDL_TEXTUREACCESS_STREAMING, width, height);
  if (!engine->texture) {
    fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
    SDL_DestroyRenderer(engine->renderer);
    SDL_DestroyWindow(engine->window);
    free(engine);
    return NULL;
  }

  return engine;
}

void engine_render_frame(Engine *eng, VirtualGPU *gpu) {
  SDL_UpdateTexture(eng->texture, NULL, gpu->framebuffer, gpu->width * 4);
  SDL_RenderClear(eng->renderer);
  SDL_RenderTexture(eng->renderer, eng->texture, NULL, NULL);
  SDL_RenderPresent(eng->renderer);
}

void engine_shutdown(Engine *eng) {
  if (!eng)
    return;
  SDL_DestroyTexture(eng->texture);
  SDL_DestroyRenderer(eng->renderer);
  SDL_DestroyWindow(eng->window);
  SDL_Quit();
  free(eng);
}
