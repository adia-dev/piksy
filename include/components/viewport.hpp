#pragma once

#include <SDL.h>
#include <SDL_pixels.h>
#include <imgui.h>

#include <memory>

namespace piksy {
namespace components {
class Viewport {
   public:
    explicit Viewport(std::shared_ptr<SDL_Renderer> renderer);
    ~Viewport();

    void render(SDL_Renderer *renderer);

   private:
    void create_render_texture(int width, int height);
    void render_selection_rect();
    void handle_viewport_click(float x, float y);
    SDL_Color get_pixel_color(int x, int y);

    std::shared_ptr<SDL_Renderer> _renderer{nullptr, SDL_DestroyRenderer};
    SDL_Texture *_render_texture = nullptr;
    ImVec2 _viewport_size;
    ImVec2 _start_dragging_position, _mouse_position;
    SDL_Rect _selection_rect;
    bool _is_dragging = false;
};
}  // namespace components
}  // namespace piksy
