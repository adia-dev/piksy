#pragma once

#include <SDL.h>
#include <SDL_pixels.h>
#include <imgui.h>

#include <components/ui_component.hpp>
#include <core/state.hpp>
#include <managers/resource_manager.hpp>
#include <rendering/renderer.hpp>
#include <vector>

#include "imgui_internal.h"

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
    void create_render_texture(int width, int height);

    void process_mouse_input();
    void process_zoom();
    void process_panning();
    void process_selection();

    void update_zoom();
    void update_pan();

    void render_cursor_hud();
    void render_placeholder_text();
    void render_texture();
    void render_grid_background();
    void render_selection_rect();
    void render_frames() const;

    // TODO: Move these to commands
    SDL_Color get_texture_pixel_color(int x, int y, const rendering::Sprite& sprite);

    void handle_click(float x, float y);

    void render_toolbar();

   private:
    rendering::Renderer& m_renderer;
    managers::ResourceManager& m_resource_manager;
    SDL_Texture* m_render_texture = nullptr;
    std::vector<rendering::Frame> m_preview_frames;
    bool m_is_previewing = false;

    ImVec2 m_viewport_size;
    SDL_Rect m_selection_rect;
    ImRect m_viewport_image_rect;
};
}  // namespace components
}  // namespace piksy
