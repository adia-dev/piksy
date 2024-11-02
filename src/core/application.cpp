#include <SDL_ttf.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

#include <components/project.hpp>
#include <core/application.hpp>
#include <core/state.hpp>
#include <iostream>
#include <managers/resource_manager.hpp>

namespace piksy {
namespace core {

Application &Application::get() {
    static Application instance;
    return instance;
}

Application::Application() { init(); }

Application::~Application() { cleanup(); }

void Application::run() {
    while (_is_running) {
        if (SDL_GetWindowFlags(_window.mutable_get()) & SDL_WINDOW_MINIMIZED) {
            SDL_Delay(10);
            continue;
        }

        handle_events();
        update();
        render();
    }
}

void Application::shutdown() {
    auto &app = get();
    app._is_running = false;
    app.cleanup();
}

rendering::Renderer &Application::mutable_renderer() { return _renderer; }

const rendering::Renderer &Application::renderer() const { return _renderer; }

void Application::init() {
    init_sdl2();
    init_imgui();
    init_textures();
    init_fonts();
    init_state();
    init_components();
}

void Application::init_sdl2() {
    if (SDL_Init(_config.init_flags) != 0) {
        printf("Error: %s\n", SDL_GetError());
        throw std::runtime_error("Error: SDL_Init()");
    }

    if (TTF_Init() < 0) {
        std::cerr << "Error initializing SDL_ttf: " << TTF_GetError() << std::endl;
        throw std::runtime_error("Error: TTF_init()");
    }

#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    _window.init(_config.window_config);
    _renderer.init(_window, _config.window_config);
}

void Application::init_imgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplSDL2_InitForSDLRenderer(_window.mutable_get(), _renderer.mutable_get());
    ImGui_ImplSDLRenderer2_Init(_renderer.mutable_get());

    _io = &ImGui::GetIO();
    (void)*_io;
    _io->ConfigFlags |= _config.imgui_config.flags;
    _io->FontGlobalScale = _config.imgui_config.font_scale;
    _io->MouseDrawCursor = _config.imgui_config.custom_mouse_cursor;

    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;

    _io->IniFilename = _config.imgui_config.ini_filename.c_str();
    printf("Loading layout from config file at: %s\n", _io->IniFilename);

    _config.imgui_config.config_style();
}

void Application::init_textures() {
    managers::ResourceManager::get().load_texture(
        _renderer.mutable_get(), std::string(RESOURCE_DIR) + "/textures/janemba.png");
}

void Application::init_fonts() {
    managers::ResourceManager::get().load_font(
        _renderer.mutable_get(), std::string(RESOURCE_DIR) + "/fonts/PixelifySans-Regular.ttf");
}

void Application::init_state() {
    auto texture = managers::ResourceManager::get().get_texture(
        _renderer.mutable_get(), std::string(RESOURCE_DIR) + "/textures/janemba.png");

    _state.texture_sprite = rendering::Sprite(texture);
}

void Application::init_components() {
    _viewport = std::make_unique<components::Viewport>(_renderer);
}

void Application::cleanup() {
    managers::ResourceManager::get().cleanup();

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    TTF_Quit();

    SDL_DestroyRenderer(_renderer.mutable_get());
    SDL_DestroyWindow(_window.mutable_get());
    SDL_Quit();

    _io = nullptr;

    std::cout << "Application successfully cleaned up\n";
}

void Application::handle_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) _is_running = false;
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
            event.window.windowID == SDL_GetWindowID(_window.mutable_get())) {
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
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

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

    _viewport->render(_renderer.mutable_get(), _state);

    ImGui::Begin("Inspector");
    ImGui::End();

    _projectComponent.render();

    ImGui::ShowDemoWindow(&_show_demo_window);

    ImGui::Render();
    // BUG: This is causing issues with SDL_WINDOW_ALLOW_HIGHDPI screen
    SDL_RenderSetScale(_renderer.mutable_get(), _io->DisplayFramebufferScale.x,
                       _io->DisplayFramebufferScale.y);
    SDL_SetRenderDrawColor(_renderer.mutable_get(), (Uint8)(_clear_color.x * 255),
                           (Uint8)(_clear_color.y * 255), (Uint8)(_clear_color.z * 255),
                           (Uint8)(_clear_color.w * 255));
    SDL_RenderClear(_renderer.mutable_get());
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), _renderer.mutable_get());
    SDL_RenderPresent(_renderer.mutable_get());
}

}  // namespace core
}  // namespace piksy
