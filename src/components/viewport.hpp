#pragma once

#include "imgui.h"
#include "sprite/sprite.hpp"
#include <SDL.h>
#include <vector>

namespace piksy {
class Viewport {
public:
  Viewport(SDL_Renderer *renderer);
  ~Viewport();

  void render(SDL_Renderer *renderer, const std::vector<Sprite> &sprites);

private:
  void create_render_texture(SDL_Renderer *renderer, int width, int height);

  SDL_Texture *_render_texture;
  ImVec2 _viewport_size;
};
} // namespace piksy
