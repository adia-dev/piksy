#include "application.hpp"
#include "components/assets.hpp"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "resource_manager/resource_manager.hpp"
#include <components/inspector.hpp>
#include <iostream>
#include <state/state.hpp>
#include <stdexcept>

namespace piksy {

Application::~Application() { cleanup(); }

void Application::run() {
  auto &app = get();

  app.init();

  while (app._is_running) {
    if (SDL_GetWindowFlags(app._window.get()) & SDL_WINDOW_MINIMIZED) {
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

  _window.reset(SDL_CreateWindow("Piksy", SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED, _window_width,
                                 _window_height, window_flags));
  if (_window == nullptr) {
    printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
    throw std::runtime_error("Error: SDL_CreateWindow()");
  }

  _renderer.reset(SDL_CreateRenderer(
      _window.get(), -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED));

  if (_renderer == nullptr) {
    SDL_Log("Error creating SDL_Renderer!");
    throw std::runtime_error("Error: SDL_CreateRenderer()");
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  _io = &ImGui::GetIO();
  (void)*_io;
  _io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  _io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  ImGui::StyleColorsDark();

  ImGui_ImplSDL2_InitForSDLRenderer(_window.get(), _renderer.get());
  ImGui_ImplSDLRenderer2_Init(_renderer.get());

  init_textures();
  init_sprites();

  _viewport = std::make_unique<Viewport>(_renderer.get());
}

void Application::init_textures() {
  ResourceManager::load_texture(_renderer.get(), std::string(RESOURCE_DIR) +
                                                     "/textures/janemba.png");
}

void Application::init_sprites() {
  State::add_sprite(Sprite(ResourceManager::get_texture(
      _renderer.get(), std::string(RESOURCE_DIR) + "/textures/janemba.png")));
}

void Application::cleanup() {
  if (_cleaned_up) {
    return;
  }

  ImGui_ImplSDLRenderer2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_DestroyRenderer(_renderer.get());
  SDL_DestroyWindow(_window.get());
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
        event.window.windowID == SDL_GetWindowID(_window.get())) {
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

    if (_io->WantCaptureMouse) {
      continue;
    }
  }
}

void Application::update() {}

void Application::render() {
  ImGui_ImplSDLRenderer2_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  {
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                    ImGuiWindowFlags_NoBringToFrontOnFocus |
                    ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("MainDockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(2);

    ImGuiIO &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
      ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
      ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    ImGui::End();
  }

  _viewport->render(_renderer.get());

  Inspector::render();
  Assets::render();
  ImGui::ShowDemoWindow(&_show_demo_window);

  ImGui::Render();
  SDL_RenderSetScale(_renderer.get(), _io->DisplayFramebufferScale.x,
                     _io->DisplayFramebufferScale.y);
  SDL_SetRenderDrawColor(_renderer.get(), (Uint8)(_clear_color.x * 255),
                         (Uint8)(_clear_color.y * 255),
                         (Uint8)(_clear_color.z * 255),
                         (Uint8)(_clear_color.w * 255));
  SDL_RenderClear(_renderer.get());
  ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), _renderer.get());
  SDL_RenderPresent(_renderer.get());
}
} // namespace piksy
