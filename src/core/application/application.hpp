#pragma once

#include "imgui.h"
#include <SDL.h>

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

  Application &operator=(Application &&) = delete;
  Application &operator=(const Application &) = delete;
  Application(Application &&) = delete;
  Application(const Application &) = delete;

  SDL_Window *_window = nullptr;
  SDL_Renderer *_renderer = nullptr;
  ImGuiIO *_io = nullptr;

  bool _show_demo_window = true;
  bool _show_another_window = false;
  bool _is_running = true;
  ImVec4 _clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  void init();
  void cleanup();

  void handle_events();
  void update();
  void render();
};
} // namespace piksy
