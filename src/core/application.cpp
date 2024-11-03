#include <SDL_ttf.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

#include <components/console.hpp>
#include <components/inspector.hpp>
#include <components/project.hpp>
#include <core/application.hpp>
#include <core/state.hpp>
#include <managers/resource_manager.hpp>
#include <memory>

#include "components/viewport.hpp"
#include "core/logger.hpp"

namespace piksy {
namespace core {

Application::Application() : _renderer(), _resource_manager(_renderer) { init(); }

Application::~Application() { cleanup(); }

void Application::run() {
    while (_is_running) {
        if (SDL_GetWindowFlags(_window.get()) & SDL_WINDOW_MINIMIZED) {
            SDL_Delay(10);
            continue;
        }

        handle_events();
        update();
        render();
    }
}

void Application::shutdown() {
    _is_running = false;

    core::Logger::debug("Shutting down the application...");
}

rendering::Renderer &Application::mutable_renderer() { return _renderer; }

const rendering::Renderer &Application::renderer() const { return _renderer; }

void Application::init() {
    Logger::get().init(&_config.logger_config);
    Logger::info("Initializing the application...");

    init_sdl2();
    init_imgui();
    init_textures();
    init_fonts();
    init_state();
    init_components();

    Logger::info("Successfully initialized the application !");
}

void Application::init_sdl2() {
    if (SDL_Init(_config.init_flags) != 0) {
        core::Logger::fatal("Error initializing SDL: %s", SDL_GetError());
    }

    if (TTF_Init() < 0) {
        core::Logger::fatal("Error initializing SDL_ttf: %s", TTF_GetError());
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

    ImGui_ImplSDL2_InitForSDLRenderer(_window.get(), _renderer.get());
    ImGui_ImplSDLRenderer2_Init(_renderer.get());

    _io = &ImGui::GetIO();
    (void)*_io;
    _io->ConfigFlags |= _config.imgui_config.flags;
    _io->FontGlobalScale = _config.imgui_config.font_scale;
    _io->MouseDrawCursor = _config.imgui_config.custom_mouse_cursor;

    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;

    _io->IniFilename = _config.imgui_config.ini_filename.c_str();
    core::Logger::debug("Loading layout from config file at: %s", _io->IniFilename);
    /* _io->Fonts->Clear(); */
    /* _io->Fonts->AddFontFromFileTTF(_config.imgui_config.font_filename.c_str(), 14.0f); */
    /* _io->Fonts->Build(); */

    _config.imgui_config.config_style();
}

void Application::init_textures() {
    _resource_manager.load_texture(std::string(RESOURCE_DIR) + "/textures/janemba.png");
}

void Application::init_fonts() {
    _resource_manager.load_font(std::string(RESOURCE_DIR) + "/fonts/PixelifySans-Regular.ttf");
}

void Application::init_state() {}

void Application::init_components() {
    _ui_components.emplace("Viewport",
                           std::make_unique<components::Viewport>(_renderer, _resource_manager));
    _ui_components.emplace("Console", std::make_unique<components::Console>());
    _ui_components.emplace("Inspector", std::make_unique<components::Inspector>());
    _ui_components.emplace("Project", std::make_unique<components::Project>(_resource_manager));
}

void Application::cleanup() {
    _resource_manager.cleanup();

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    TTF_Quit();

    SDL_DestroyRenderer(_renderer.get());
    SDL_DestroyWindow(_window.get());
    SDL_Quit();

    _io = nullptr;

    core::Logger::debug("Application successfully cleaned up");
}

void Application::handle_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) {
            _is_running = false;
        } else if (event.type == SDL_DROPFILE) {
            const char *dropped_filedir = event.drop.file;

            ((components::Viewport *)_ui_components["Viewport"].get())
                ->notify_dropped_file(_state, dropped_filedir);

            SDL_free((void *)dropped_filedir);
        }

        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
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

void Application::update() {
    for (auto &[name, component] : _ui_components) {
        component->update();
    }
}

void Application::render() {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    {
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar;

        ImGuiViewport *viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);

        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("MainDockSpace", nullptr, window_flags);
        ImGui::PopStyleVar();

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open...", "Ctrl+O")) {
                    // TODO: Handle opening a project
                }
                if (ImGui::MenuItem("Save", "Ctrl+S")) {
                    // TODO: Handle saving the project
                }
                if (ImGui::MenuItem("Exit", "Alt+F4")) {
                    shutdown();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                // TODO: Add edit menu items here
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGuiIO &io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGuiID dockspace_id = ImGui::GetID("MainDockSpaceID");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        ImGui::End();
    }

    for (auto &[name, component] : _ui_components) {
        component->render(_state);
    }

    // ImGui::ShowDemoWindow(&_show_demo_window);

    ImGui::Render();
    SDL_RenderSetScale(_renderer.get(), _io->DisplayFramebufferScale.x,
                       _io->DisplayFramebufferScale.y);
    SDL_SetRenderDrawColor(_renderer.get(), static_cast<Uint8>(_clear_color.x * 255),
                           static_cast<Uint8>(_clear_color.y * 255),
                           static_cast<Uint8>(_clear_color.z * 255),
                           static_cast<Uint8>(_clear_color.w * 255));
    SDL_RenderClear(_renderer.get());
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), _renderer.get());
    SDL_RenderPresent(_renderer.get());
}

}  // namespace core
}  // namespace piksy
