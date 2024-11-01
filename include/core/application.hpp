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
    std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> _renderer{nullptr,
                                                                            SDL_DestroyRenderer};
    ImGuiIO *_io = nullptr;

    int _window_width = 1080;
    int _window_height = 720;

    bool _show_demo_window = true;
    bool _is_running = true;
    bool _cleaned_up = false;
    ImVec4 _clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    std::unique_ptr<Viewport> _viewport{nullptr};

    // Components
    Project _projectComponent;

    void init();
    void init_textures();
    void init_sprites();
    void cleanup();

    void handle_events();
    void update();
    void render();
};
}  // namespace piksy
