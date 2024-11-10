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

    State &mutable_state() { return _state; }
    const State &state() const { return _state; }

    const Config &config() const { return _config; }

    rendering::Renderer &mutable_renderer();
    const rendering::Renderer &renderer() const;

   private:
    Application &operator=(Application &&) = delete;
    Application &operator=(const Application &) = delete;
    Application(Application &&) = delete;
    Application(const Application &) = delete;

    rendering::Window _window;
    rendering::Renderer _renderer;
    managers::ResourceManager _resource_manager;

    ImGuiIO *_io = nullptr;

    // TODO: move this into the state
    bool _show_demo_window = true;
    bool _is_running = true;
    ImVec4 _clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    Config _config;
    State _state;

    std::unordered_map<std::string, std::unique_ptr<components::UIComponent>> _ui_components;

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
};
}  // namespace core
}  // namespace piksy
