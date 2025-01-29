#pragma once

#include <contexts/imgui_context.hpp>
#include <contexts/sdl_context.hpp>
#include <core/config.hpp>
#include <core/state.hpp>
#include <managers/resource_manager.hpp>
#include <rendering/renderer.hpp>
#include <rendering/window.hpp>

#include "layers/layer_stack.hpp"
#include "managers/animation_manager.hpp"

namespace piksy {
namespace core {

class Application {
   public:
    Application();
    ~Application();

    void run();
    void shutdown();

    State &mutable_state() { return m_state; }
    const State &state() const { return m_state; }

    const Config &config() const { return m_config; }

    rendering::Renderer &mutable_renderer();
    const rendering::Renderer &renderer() const;

   private:
    // NOTE: too many init methods, code smell like sh*t
    void init();

    // This is smelly
    void init_textures();
    void init_fonts();
    void init_state();

    void cleanup();

    void handle_events();
    void update();
    void render();

    void set_fancy_imgui_style();

   private:
    Application &operator=(Application &&) = delete;
    Application &operator=(const Application &) = delete;
    Application(Application &&) = delete;
    Application(const Application &) = delete;

   private:
    rendering::Window m_window;
    rendering::Renderer m_renderer;
    managers::ResourceManager m_resource_manager;
    managers::AnimationManager m_animation_manager;

    ImGuiIO *m_io = nullptr;

    // TODO: move this into the state
    bool m_show_demo_window = true;
    bool m_is_running = true;
    ImVec4 m_clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    contexts::SDLContext m_sdl_system;
    contexts::ImGuiContext m_gui_system;

    layers::LayerStack m_layer_stack;

    Config m_config;
    State m_state;
};
}  // namespace core
}  // namespace piksy
