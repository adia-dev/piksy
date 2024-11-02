#pragma once

#include <SDL.h>
#include <SDL_pixels.h>
#include <imgui.h>

#include <core/state.hpp>
#include <rendering/renderer.hpp>

namespace piksy {
namespace components {
class Viewport {
   public:
    explicit Viewport(rendering::Renderer& renderer);
    ~Viewport();

    void render(SDL_Renderer* renderer, core::State& state);

   private:
    void create_render_texture(int width, int height);
    void render_selection_rect();
    void handle_viewport_click(float x, float y, core::State& state);
    SDL_Color get_pixel_color(int x, int y);

    // replace to pointer to RAII
    rendering::Renderer& _renderer;
    SDL_Texture* _render_texture = nullptr;

    ImVec2 _viewport_size;
    ImVec2 _start_dragging_position, _mouse_position;
    SDL_Rect _selection_rect;
    bool _is_dragging = false;
};
}  // namespace components
}  // namespace piksy
