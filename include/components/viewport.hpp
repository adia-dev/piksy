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
    explicit Viewport(core::State& state, rendering::Renderer& renderer,
                      managers::ResourceManager& resource_manager);
    ~Viewport();

    void update() override;
    void render() override;

    void notify_dropped_file(const std::string& dropped_file_path);

   private:
    // Processing methods
    void handle_mouse_input();
    void process_zoom();
    void process_panning();
    void update_zoom();
    void update_pan();
    void process_selection();

    // Rendering methods
    void create_render_texture(int width, int height);
    void render_placeholder_text();
    void render_texture();
    void render_grid_background();
    void render_selection_rect();
    void render_frames(const std::vector<SDL_Rect>& frames) const;

    // Utility methods
    void handle_viewport_click(float x, float y);
    SDL_Color get_texture_pixel_color(int x, int y, const rendering::Sprite& sprite);
    void swap_texture_color(const SDL_Color& from, const SDL_Color& to);

    rendering::Renderer& _renderer;
    managers::ResourceManager& _resource_manager;
    SDL_Texture* _render_texture = nullptr;

    ImVec2 _viewport_size;
    SDL_Rect _selection_rect;
};
}  // namespace components
}  // namespace piksy
