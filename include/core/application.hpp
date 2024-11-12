#pragma once

#include <SDL.h>
#include <SDL2/SDL_ttf.h>
#include <imgui.h>

#include <components/project.hpp>
#include <components/ui_component.hpp>
#include <components/viewport.hpp>
#include <core/config.hpp>
#include <core/state.hpp>
#include <memory>
#include <rendering/renderer.hpp>
#include <rendering/window.hpp>
#include <unordered_map>

#include "managers/resource_manager.hpp"

#if !SDL_VERSION_ATLEAST(2, 0, 17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

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

    // TODO: Create a Facade of those two classes in order to initialize them
    void init_sdl2();
    void init_imgui();

    // This is smelly
    void init_textures();
    void init_fonts();
    void init_state();
    void init_components();

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

    rendering::Window m_window;
    rendering::Renderer m_renderer;
    managers::ResourceManager m_resource_manager;

    ImGuiIO *m_io = nullptr;

    // TODO: move this into the state
    bool m_show_demo_window = true;
    bool m_is_running = true;
    ImVec4 m_clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    Config m_config;
    State m_state;

    std::unordered_map<std::string, std::unique_ptr<components::UIComponent>> m_ui_components;
};
}  // namespace core
}  // namespace piksy
