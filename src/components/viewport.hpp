#pragma once

#include "imgui.h"
#include <SDL.h>

namespace piksy {
class Viewport {
public:
  explicit Viewport(SDL_Renderer *renderer);
  ~Viewport();

  void render(SDL_Renderer *renderer);

private:
  void create_render_texture(SDL_Renderer *renderer, int width, int height);
  void handle_viewport_click(float x, float y);

  SDL_Texture *_render_texture;
  ImVec2 _viewport_size;
};
} // namespace piksy
