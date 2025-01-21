#include <SDL_render.h>
#include <SDL_ttf.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <imgui_internal.h>

#include <command/export_animation_command.hpp>
#include <command/export_texture_command.hpp>
#include <command/load_command.hpp>
#include <command/save_command.hpp>
#include <core/application.hpp>
#include <core/config.hpp>
#include <core/logger.hpp>
#include <core/state.hpp>
#include <layers/editor_layer.hpp>
#include <managers/resource_manager.hpp>
#include <memory>
#include <string>

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
    Logger::init(&m_config.logger_config);
    Logger::info("Initializing the application...");

    m_sdl_system.init(m_config.window_config);
    m_window.init(m_config.window_config);
    m_renderer.init(m_window, m_config.window_config);
    m_gui_system.init(m_config.imgui_config, m_window, m_renderer);

    m_io = &ImGui::GetIO();
    (void)*m_io;

    init_textures();
    init_fonts();
    init_state();

    m_layer_stack.push_layer<layers::EditorLayer>(m_renderer, m_resource_manager, m_state);

    Logger::info("Successfully initialized the application !");
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

void Application::cleanup() {
    m_resource_manager.cleanup();

    m_gui_system.cleanup();
    m_sdl_system.cleanup(m_window, m_renderer);
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
        }

        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
            event.window.windowID == SDL_GetWindowID(m_window.get())) {
            // TODO: Configure this `Save on Exit`
            commands::SaveCommand command(m_state, m_config.app_config.save_file);
            command.execute();

            m_is_running = false;
        }

        bool event_handled = false;
        for (auto &layer : m_layer_stack.layers()) {
            layer->on_event(event, event_handled);

            if (event_handled) {
                break;
            }
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

    for (auto &layer : m_layer_stack.layers()) {
        layer->on_update(delta_time);
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

    for (auto &layer : m_layer_stack.layers()) {
        layer->on_render();
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

        ImVec4 color = Logger::messages().empty()
                           ? ImVec4(0.0f, 0.0f, 0.0f, 255.0f)
                           : LogLevelToColor(Logger::messages().back().first);
        ImGui::PushStyleColor(ImGuiCol_MenuBarBg, color);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 255));
        if (ImGui::BeginViewportSideBar("##MainStatusBar", viewport, ImGuiDir_Down, height,
                                        window_flags)) {
            if (ImGui::BeginMenuBar()) {
                if (!Logger::messages().empty()) {
                    auto &[level, message] = Logger::messages().back();
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
