#include "application.hpp"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include <stdexcept>

namespace piksy {

void Application::run() {
  auto &app = get();

  app.init();

  while (app._is_running) {
    if (SDL_GetWindowFlags(app._window) & SDL_WINDOW_MINIMIZED) {
      SDL_Delay(10);
      continue;
    }

    app.handle_events();
    app.update();
    app.render();
  }

  app.cleanup();
}

void Application::shutdown() {
  auto &app = get();
  app._is_running = false;
  app.cleanup();
}

void Application::init() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) !=
      0) {
    printf("Error: %s\n", SDL_GetError());
    throw std::runtime_error("Error: SDL_Init()");
  }

#ifdef SDL_HINT_IME_SHOW_UI
  SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

  SDL_WindowFlags window_flags =
      (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

  _window = SDL_CreateWindow("Piksy", SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
  if (_window == nullptr) {
    printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
    throw std::runtime_error("Error: SDL_CreateWindow()");
  }

  _renderer = SDL_CreateRenderer(
      _window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
  if (_renderer == nullptr) {
    SDL_Log("Error creating SDL_Renderer!");
    throw std::runtime_error("Error: SDL_CreateRenderer()");
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  _io = &ImGui::GetIO();
  (void)*_io;

  ImGui::StyleColorsDark();

  ImGui_ImplSDL2_InitForSDLRenderer(_window, _renderer);
  ImGui_ImplSDLRenderer2_Init(_renderer);
}

void Application::cleanup() {
  ImGui_ImplSDLRenderer2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_DestroyRenderer(_renderer);
  SDL_DestroyWindow(_window);
  SDL_Quit();

  _renderer = nullptr;
  _window = nullptr;
  _io = nullptr;
}

void Application::handle_events() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent(&event);
    if (event.type == SDL_QUIT)
      _is_running = false;
    if (event.type == SDL_WINDOWEVENT &&
        event.window.event == SDL_WINDOWEVENT_CLOSE &&
        event.window.windowID == SDL_GetWindowID(_window)) {
      _is_running = true;
    }

    if (event.type == SDL_KEYUP) {
      switch (event.key.keysym.sym) {
      case SDLK_ESCAPE:
        _is_running = false;
      default:
        break;
      }
    }
  }
}

void Application::update() {}

void Application::render() {
  ImGui_ImplSDLRenderer2_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  if (_show_demo_window)
    ImGui::ShowDemoWindow(&_show_demo_window);

  {
    static float f = 0.0f;
    static int counter = 0;

    ImGui::Begin("Hello, world!");

    ImGui::Text("This is some useful text.");

    ImGui::Checkbox("Demo Window", &_show_demo_window);
    ImGui::Checkbox("Another Window", &_show_another_window);

    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
    ImGui::ColorEdit3("clear color", (float *)&_clear_color);

    if (ImGui::Button("Button"))
      counter++;

    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / _io->Framerate, _io->Framerate);
    ImGui::End();
  }

  if (_show_another_window) {
    ImGui::Begin("Another Window", &_show_another_window);

    ImGui::Text("Hello from another window!");
    if (ImGui::Button("Close Me"))
      _show_another_window = false;
    ImGui::End();
  }

  ImGui::Render();
  SDL_RenderSetScale(_renderer, _io->DisplayFramebufferScale.x,
                     _io->DisplayFramebufferScale.y);
  SDL_SetRenderDrawColor(
      _renderer, (Uint8)(_clear_color.x * 255), (Uint8)(_clear_color.y * 255),
      (Uint8)(_clear_color.z * 255), (Uint8)(_clear_color.w * 255));
  SDL_RenderClear(_renderer);
  ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), _renderer);
  SDL_RenderPresent(_renderer);
}

} // namespace piksy
