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
  void render_selection_rect(SDL_Renderer *renderer);
  void handle_viewport_click(float x, float y);


  SDL_Texture *_render_texture = nullptr;
  ImVec2 _viewport_size;
  ImVec2 _start_dragging_position, _mouse_position;
  SDL_Rect _selection_rect;
  bool _is_dragging = false;
};
} // namespace piksy
