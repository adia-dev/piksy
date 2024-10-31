#pragma once

#include "imgui.h"
#include "sprite/sprite.hpp"
#include "texture/texture2D.hpp"
#include <SDL.h>
#include <unordered_map>
#include <vector>

#if !SDL_VERSION_ATLEAST(2, 0, 17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

namespace piksy {
class Application {
public:
  static Application &get() {
    static Application instance;
    return instance;
  }

  static void run();
  static void shutdown();

private:
  Application() = default;
  ~Application();

  Application &operator=(Application &&) = delete;
  Application &operator=(const Application &) = delete;
  Application(Application &&) = delete;
  Application(const Application &) = delete;

  SDL_Window *_window = nullptr;
  SDL_Renderer *_renderer = nullptr;
  ImGuiIO *_io = nullptr;

  std::vector<Sprite> _sprites;

  int _window_width = 1920;
  int _window_height = 720;

  bool _show_demo_window = true;
  bool _is_running = true;
  bool _cleaned_up = false;
  ImVec4 _clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  void init();
  void init_textures();
  void init_sprites();
  void cleanup();

  void handle_events();
  void update();
  void render();
};
} // namespace piksy
