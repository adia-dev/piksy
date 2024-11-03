#pragma once

#include <SDL.h>
#include <SDL_pixels.h>
#include <imgui.h>

#include <components/ui_component.hpp>
#include <core/state.hpp>
#include <rendering/renderer.hpp>

namespace piksy {
namespace components {

class Viewport : public UIComponent {
   public:
    explicit Viewport(rendering::Renderer& renderer);
    ~Viewport();

    void update() override;
    void render(core::State& state) override;

   private:
    void create_render_texture(int width, int height);
    void handle_viewport_click(float x, float y, core::State& state);

    void render_selection_rect();
    void render_grid_background();

    SDL_Color get_pixel_color(int x, int y);

    void process_zoom();
    void process_panning();
    void update_zoom();
    void update_pan();
    void handle_mouse_input(core::State& state);

    // State structures
    struct MouseState {
        ImVec2 start_pos;
        ImVec2 current_pos;
        bool is_pressed = false;
        bool is_panning = false;
    };

    struct ZoomState {
        float current_scale = 1.0f;
        float target_scale = 1.0f;
        float zoom_speed = 0.1f;
    };

    struct PanState {
        ImVec2 current_offset;
        ImVec2 target_offset;
        float pan_speed = 0.7f;
    };

    rendering::Renderer& _renderer;
    SDL_Texture* _render_texture = nullptr;

    ImVec2 _viewport_size;
    SDL_Rect _selection_rect;
    int _grid_cell_size = 20;

    MouseState _mouse_state;
    ZoomState _zoom_state;
    PanState _pan_state;

    ImVec2 lerp(const ImVec2& lhs, const ImVec2& rhs, float t);
    float lerp(float lhs, float rhs, float t);
};

}  // namespace components
}  // namespace piksy
