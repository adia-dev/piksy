#pragma once

#include <SDL.h>
#include <imgui.h>

#include <components/project.hpp>
#include <components/viewport.hpp>
#include <memory>

#if !SDL_VERSION_ATLEAST(2, 0, 17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

namespace piksy {
namespace core {
class Application {
   public:
    static Application &get();

    static void run();
    static void shutdown();

   private:
    Application() = default;
    ~Application();

    Application &operator=(Application &&) = delete;
    Application &operator=(const Application &) = delete;
    Application(Application &&) = delete;
    Application(const Application &) = delete;

    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> _window{nullptr, SDL_DestroyWindow};
    std::shared_ptr<SDL_Renderer> _renderer{nullptr, SDL_DestroyRenderer};
    ImGuiIO *_io = nullptr;

    int _window_width = 1440;
    int _window_height = 900;

    bool _show_demo_window = true;
    bool _is_running = true;
    bool _cleaned_up = false;
    ImVec4 _clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    std::unique_ptr<components::Viewport> _viewport{nullptr};

    // Components
    components::Project _projectComponent;

    void init();
    void init_sdl2();
    void init_imgui();
    void init_textures();
    void init_sprites();
    void init_components();
    void cleanup();

    void handle_events();
    void update();
    void render();

    void set_fancy_imgui_style();
};
}  // namespace core
}  // namespace piksy
