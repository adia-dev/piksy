#include <SDL_ttf.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <imgui_internal.h>

#include <components/console.hpp>
#include <components/frame_viewer.hpp>
#include <components/inspector.hpp>
#include <components/project.hpp>
#include <components/viewport.hpp>
#include <core/application.hpp>
#include <core/config.hpp>
#include <core/logger.hpp>
#include <core/state.hpp>
#include <managers/resource_manager.hpp>
#include <memory>
#include <string>

#include "SDL_render.h"
#include "command/export_animation_command.hpp"
#include "command/export_texture_command.hpp"
#include "command/load_command.hpp"
#include "command/save_command.hpp"
#include "components/animation_preview.hpp"

namespace piksy {
namespace core {

Application::Application() : m_renderer(), m_resource_manager(m_renderer) { init(); }

Application::~Application() { cleanup(); }

void Application::run() {
    while (m_is_running) {
        if (SDL_GetWindowFlags(m_window.get()) & SDL_WINDOW_MINIMIZED) {
            SDL_Delay(10);
            continue;
        }

        handle_events();
        update();
        render();
    }
}

void Application::shutdown() {
    m_is_running = false;

    core::Logger::debug("Shutting down the application...");
}

rendering::Renderer &Application::mutable_renderer() { return m_renderer; }

const rendering::Renderer &Application::renderer() const { return m_renderer; }

void Application::init() {
    Logger::get().init(&m_config.logger_config);
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
    if (SDL_Init(m_config.init_flags) != 0) {
        core::Logger::fatal("Error initializing SDL: %s", SDL_GetError());
    }

    if (TTF_Init() < 0) {
        core::Logger::fatal("Error initializing SDL_ttf: %s", TTF_GetError());
    }

#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    m_window.init(m_config.window_config);
    m_renderer.init(m_window, m_config.window_config);
}

void Application::init_imgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplSDL2_InitForSDLRenderer(m_window.get(), m_renderer.get());
    ImGui_ImplSDLRenderer2_Init(m_renderer.get());

    m_io = &ImGui::GetIO();
    (void)*m_io;
    m_io->ConfigFlags |= m_config.imgui_config.flags;
    m_io->FontGlobalScale = m_config.imgui_config.font_scale;
    m_io->MouseDrawCursor = m_config.imgui_config.custom_mouse_cursor;

    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;

    m_io->IniFilename = m_config.imgui_config.ini_filename.c_str();
    core::Logger::debug("Loading layout from config file at: %s", m_io->IniFilename);
    /* _io->Fonts->Clear(); */
    /* _io->Fonts->AddFontFromFileTTF(_config.imgui_config.font_filename.c_str(), 14.0f); */
    /* _io->Fonts->Build(); */

    m_config.imgui_config.config_style();
}

void Application::init_textures() {
    m_resource_manager.load_texture(std::string(RESOURCE_DIR) + "/textures/janemba.png");
}

void Application::init_fonts() {
    m_resource_manager.load_font(std::string(RESOURCE_DIR) + "/fonts/PixelifySans-Regular.ttf");
}

void Application::init_state() {
    {
        // NOTE: DON'T COMMIT
        // UPDATE: ...
        commands::LoadCommand command(m_state, m_resource_manager, m_config.app_config.save_file);
        command.execute();
    }

    if (m_state.animation_state.animations.empty()) {
        m_state.animation_state.current_animation = "Untitled";
        m_state.animation_state.animations["Untitled"];
    } else if (m_state.animation_state.current_animation.empty()) {
        m_state.animation_state.current_animation =
            m_state.animation_state.animations.begin()->first;
    }
}

void Application::init_components() {
    m_ui_components.emplace("Viewport", std::make_unique<components::Viewport>(m_state, m_renderer,
                                                                               m_resource_manager));
    m_ui_components.emplace("Console", std::make_unique<components::Console>(m_state));
    /* _ui_components.emplace("Inspector", std::make_unique<components::Inspector>(_state,
     * _renderer)); */
    m_ui_components.emplace("Project",
                            std::make_unique<components::Project>(m_state, m_resource_manager));
    m_ui_components.emplace("Frame Viewer", std::make_unique<components::FrameViewer>(m_state));
    m_ui_components.emplace("Animation Preview",
                            std::make_unique<components::AnimationPreview>(m_state));
}

void Application::cleanup() {
    m_resource_manager.cleanup();

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    TTF_Quit();

    SDL_DestroyRenderer(m_renderer.get());
    SDL_DestroyWindow(m_window.get());
    SDL_Quit();

    m_io = nullptr;

    core::Logger::debug("Application successfully cleaned up");
}

void Application::handle_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) {
            // TODO: Configure this `Save on Exit`
            commands::SaveCommand command(m_state, m_config.app_config.save_file);
            command.execute();

            m_is_running = false;
        } else if (event.type == SDL_DROPFILE) {
            const char *dropped_filedir = event.drop.file;

            reinterpret_cast<components::Viewport *>(m_ui_components["Viewport"].get())
                ->notify_dropped_file(dropped_filedir);

            SDL_free((void *)dropped_filedir);
        }

        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
            event.window.windowID == SDL_GetWindowID(m_window.get())) {
            // TODO: Configure this `Save on Exit`
            commands::SaveCommand command(m_state, m_config.app_config.save_file);
            command.execute();

            m_is_running = false;
        }

        if (m_io->WantCaptureMouse) {
            continue;
        }
    }
}

void Application::update() {
    static Uint64 last_frame_time = SDL_GetPerformanceCounter();
    Uint64 current_frame_time = SDL_GetPerformanceCounter();
    float delta_time =
        (current_frame_time - last_frame_time) / static_cast<float>(SDL_GetPerformanceFrequency());
    last_frame_time = current_frame_time;

    // Update delta time and fps in the state for global access
    m_state.delta_time = delta_time;
    m_state.fps = 1.0f / delta_time;

    for (auto &[name, component] : m_ui_components) {
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

        ImGuiIO &io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGuiID dockspace_id = ImGui::GetID("MainDockSpaceID");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        ImGui::End();
    }

    for (auto &[name, component] : m_ui_components) {
        component->render();
    }

    {
        ImGuiViewportP *viewport = (ImGuiViewportP *)(void *)ImGui::GetMainViewport();
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar |
                                        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
        float height = ImGui::GetFrameHeight();

        if (ImGui::BeginViewportSideBar("##SecondaryMenuBar", viewport, ImGuiDir_Up, height,
                                        window_flags)) {
            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Open...", "Ctrl+O")) {
                        commands::LoadCommand command(m_state, m_resource_manager,
                                                      m_config.app_config.save_file);
                        command.execute();
                    }
                    if (ImGui::MenuItem("Save", "Cmd+S")) {
                        commands::SaveCommand command(m_state, m_config.app_config.save_file);
                        command.execute();
                    }

                    if (ImGui::MenuItem("Export Animations...", nullptr)) {
                        // Hard-code or ask user for path
                        // e.g. auto export_path = "./animations_export.json";
                        // or open an "ImGuiFileDialog" or something
                        std::string export_path = "./export_animations.json";

                        commands::ExportAnimationsCommand export_cmd(m_state, export_path);
                        export_cmd.execute();
                    }

                    if (ImGui::MenuItem("Export Texture as PNG...")) {
                        fs::path export_path = "./exported_texture.png";

                        commands::ExportTextureCommand export_cmd(m_state, export_path);
                        export_cmd.execute();
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
                if (ImGui::BeginMenu("Tools")) {
                    // TODO: Add tools menu items here
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Help")) {
                    // TODO: Add tools menu items here
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("About Piksy")) {
                    if (ImGui::MenuItem("Open...", "Ctrl+O")) {
                        // TODO: Handle opening a project
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
            ImGui::End();
        }

        ImVec4 color = Logger::get().messages().empty()
                           ? ImVec4(0.0f, 0.0f, 0.0f, 255.0f)
                           : LogLevelToColor(Logger::get().messages().back().first);
        ImGui::PushStyleColor(ImGuiCol_MenuBarBg, color);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 255));
        if (ImGui::BeginViewportSideBar("##MainStatusBar", viewport, ImGuiDir_Down, height,
                                        window_flags)) {
            if (ImGui::BeginMenuBar()) {
                if (!Logger::get().messages().empty()) {
                    auto &[level, message] = Logger::get().messages().back();
                    ImGui::Text("%s", message.c_str());
                }
                ImGui::EndMenuBar();
            }
            ImGui::End();
        }
        ImGui::PopStyleColor(2);
    }

    /* ImGui::ShowDemoWindow(&_show_demo_window); */

    ImGui::Render();
    SDL_RenderSetScale(m_renderer.get(), m_io->DisplayFramebufferScale.x,
                       m_io->DisplayFramebufferScale.y);
    SDL_SetRenderDrawColor(m_renderer.get(), static_cast<Uint8>(m_clear_color.x * 255),
                           static_cast<Uint8>(m_clear_color.y * 255),
                           static_cast<Uint8>(m_clear_color.z * 255),
                           static_cast<Uint8>(m_clear_color.w * 255));
    SDL_RenderClear(m_renderer.get());
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), m_renderer.get());
    SDL_RenderPresent(m_renderer.get());
}

}  // namespace core
}  // namespace piksy
