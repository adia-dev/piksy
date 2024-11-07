#pragma once

#include <SDL.h>
#include <SDL_pixels.h>
#include <imgui.h>

#include <components/ui_component.hpp>
#include <core/state.hpp>
#include <managers/resource_manager.hpp>
#include <rendering/renderer.hpp>
#include <vector>

namespace piksy {
namespace components {

class Viewport : public UIComponent {
   public:
    explicit Viewport(rendering::Renderer& renderer, managers::ResourceManager& resource_manager);
    ~Viewport();

    void update(core::State& state) override;
    void render(core::State& state) override;

    void notify_dropped_file(core::State& state, const std::string& dropped_file_path);

   private:
    void create_render_texture(int width, int height);
    void handle_viewport_click(float x, float y, core::State& state);
    void render_placeholder_text();

    void render_selection_rect(core::State& state);
    void render_grid_background(core::State& state);
    void render_frames(const std::vector<SDL_Rect>& frames) const;

    SDL_Color get_texture_pixel_color(int x, int y, const rendering::Sprite& sprite);

    void process_zoom();
    void process_panning();
    void update_zoom();
    void update_pan();
    void handle_mouse_input(core::State& state);
    void swap_texture_color(const SDL_Color& from, const SDL_Color& to, core::State& state);

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
    managers::ResourceManager& _resource_manager;
    SDL_Texture* _render_texture = nullptr;

    ImVec2 _viewport_size;
    SDL_Rect _selection_rect;
    std::vector<SDL_Rect> _frames;

    MouseState _mouse_state;
    ZoomState _zoom_state;
    PanState _pan_state;
};

}  // namespace components
}  // namespace piksy
