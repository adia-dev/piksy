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
    init_sdl2();
    init_imgui();
    init_textures();
    init_sprites();
    init_components();
}

void Application::init_sdl2() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        printf("Error: %s\n", SDL_GetError());
        throw std::runtime_error("Error: SDL_Init()");
    }

#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    SDL_WindowFlags window_flags =
        (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

    _window.reset(SDL_CreateWindow("Piksy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   _window_width, _window_height, window_flags));
    if (_window == nullptr) {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        throw std::runtime_error("Error: SDL_CreateWindow()");
    }

    _renderer.reset(
        SDL_CreateRenderer(_window.get(), -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED),
        SDL_DestroyRenderer);

    if (_renderer == nullptr) {
        SDL_Log("Error creating SDL_Renderer!");
        throw std::runtime_error("Error: SDL_CreateRenderer()");
    }
}

void Application::init_imgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplSDL2_InitForSDLRenderer(_window.get(), _renderer.get());
    ImGui_ImplSDLRenderer2_Init(_renderer.get());

    _io = &ImGui::GetIO();
    (void)*_io;
    _io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    _io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    /* _io->ConfigFlags |= ImGuiConfigFlags_DockingEnable; */
    _io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    /* _io->FontGlobalScale = 1.5f; */
    /* _io->MouseDrawCursor = true; */

    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;

    // TODO: Make this variable a constant and use it here
    static std::string ini_filename = std::string(RESOURCE_DIR) + "/config/window.ini";
    _io->IniFilename = ini_filename.c_str();
    printf("Loading layout from config file at: %s\n", _io->IniFilename);

    set_fancy_imgui_style();
}

// TODO: This takes a lot of space, maybe move it to the header file or in another file
void Application::set_fancy_imgui_style() {
    ImVec4 *colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
    colors[ImGuiCol_Border] = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
    colors[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
    colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.23f, 0.23f, 0.23f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.35f, 0.35f, 0.35f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.36f, 0.36f, 0.36f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.34f, 0.34f, 0.34f, 0.35f);

    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(8.00f, 8.00f);
    style.FramePadding = ImVec2(20.00f, 10.00f);
    style.CellPadding = ImVec2(6.00f, 6.00f);
    style.ItemSpacing = ImVec2(6.00f, 6.00f);
    style.ItemInnerSpacing = ImVec2(6.00f, 6.00f);
    style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
    style.IndentSpacing = 25;
    style.ScrollbarSize = 15;
    style.GrabMinSize = 10;
    style.WindowBorderSize = 0;
    style.ChildBorderSize = 0;
    style.PopupBorderSize = 0;
    style.FrameBorderSize = 0;
    style.TabBorderSize = 1;
    style.WindowRounding = 0;
    style.ChildRounding = 4;
    style.FrameRounding = 7;
    style.PopupRounding = 12;
    style.ScrollbarRounding = 9;
    style.GrabRounding = 3;
    style.LogSliderDeadzone = 4;
    style.TabRounding = 4;

    style.WindowMenuButtonPosition = ImGuiDir_None;
}

void Application::init_textures() {
    managers::ResourceManager::load_texture(_renderer.get(),
                                            std::string(RESOURCE_DIR) + "/textures/janemba.png");
}

void Application::init_sprites() {
    State::add_sprite(rendering::Sprite(managers::ResourceManager::get_texture(
        _renderer.get(), std::string(RESOURCE_DIR) + "/textures/janemba.png")));
}

void Application::init_components() {
    _viewport = std::make_unique<components::Viewport>(_renderer);
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
        if (event.type == SDL_QUIT) _is_running = false;
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

    _viewport->render(_renderer.get());

    ImGui::Begin("Inspector");
    ImGui::End();

    _projectComponent.render();

    ImGui::ShowDemoWindow(&_show_demo_window);

    ImGui::Render();
    // BUG: This is causing issues with SDL_WINDOW_ALLOW_HIGHDPI screen
    SDL_RenderSetScale(_renderer.get(), _io->DisplayFramebufferScale.x,
                       _io->DisplayFramebufferScale.y);
    SDL_SetRenderDrawColor(_renderer.get(), (Uint8)(_clear_color.x * 255),
                           (Uint8)(_clear_color.y * 255), (Uint8)(_clear_color.z * 255),
                           (Uint8)(_clear_color.w * 255));
    SDL_RenderClear(_renderer.get());
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), _renderer.get());
    SDL_RenderPresent(_renderer.get());
}

}  // namespace core
}  // namespace piksy
