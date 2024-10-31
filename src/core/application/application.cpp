#include "application.hpp"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "resource_manager/resource_manager.hpp"
#include <iostream>
#include <stdexcept>

namespace piksy {

Application::~Application() { cleanup(); }

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

  _window =
      SDL_CreateWindow("Piksy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       _window_width, _window_height, window_flags);
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

  init_textures();
  init_sprites();
}

void Application::init_textures() {
  ResourceManager::load_texture(_renderer, std::string(RESOURCE_DIR) +
                                               "/textures/janemba.png");
}

void Application::init_sprites() {
  _sprites.emplace_back(ResourceManager::get_texture(
      _renderer, std::string(RESOURCE_DIR) + "/textures/janemba.png"));
}

void Application::cleanup() {
  if (_cleaned_up) {
    return;
  }

  ImGui_ImplSDLRenderer2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_DestroyRenderer(_renderer);
  SDL_DestroyWindow(_window);
  SDL_Quit();

  _renderer = nullptr;
  _window = nullptr;
  _io = nullptr;
  _cleaned_up = true;

  std::cout << "Application successfully cleaned up\n";
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
      _is_running = false;
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
    ImGui::Begin("Sprite Inspector");

    for (size_t i = 0; i < _sprites.size(); ++i) {
      ImGui::PushID(static_cast<int>(i));

      if (ImGui::CollapsingHeader(("Sprite " + std::to_string(i)).c_str())) {
        Sprite &sprite = _sprites[i];
        ImVec2 position(sprite.x(), sprite.y());
        ImVec2 size(sprite.width(), sprite.height());
        ImVec4 frameRect(static_cast<float>(sprite.frame_rect().x),
                         static_cast<float>(sprite.frame_rect().y),
                         static_cast<float>(sprite.frame_rect().w),
                         static_cast<float>(sprite.frame_rect().h));

        ImGui::DragFloat2("Position", (float *)&position);
        ImGui::DragFloat2("Size", (float *)&size);
        ImGui::DragFloat4("Frame Rect", (float *)&frameRect);

        sprite.set_position(position.x, position.y);
        sprite.set_size(size.x, size.y);
        sprite.set_frame_rect(
            {static_cast<int>(frameRect.x), static_cast<int>(frameRect.y),
             static_cast<int>(frameRect.z), static_cast<int>(frameRect.w)});
      }

      ImGui::PopID();
    }

    ImGui::End();
  }

  ImGui::Render();
  SDL_RenderSetScale(_renderer, _io->DisplayFramebufferScale.x,
                     _io->DisplayFramebufferScale.y);
  SDL_SetRenderDrawColor(
      _renderer, (Uint8)(_clear_color.x * 255), (Uint8)(_clear_color.y * 255),
      (Uint8)(_clear_color.z * 255), (Uint8)(_clear_color.w * 255));
  SDL_RenderClear(_renderer);

  for (const Sprite &sprite : _sprites) {
    sprite.render(_renderer);
  }

  ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), _renderer);
  SDL_RenderPresent(_renderer);
}
} // namespace piksy
