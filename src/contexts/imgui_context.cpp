#include <contexts/imgui_context.hpp>

#include "core/logger.hpp"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

namespace piksy {
namespace contexts {

ImGuiContext::~ImGuiContext() {
    if (m_cleaned_up) return;

    cleanup();
}

void ImGuiContext::init(const core::ImGuiConfig& config, rendering::Window& window,
                        rendering::Renderer& renderer) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplSDL2_InitForSDLRenderer(window.get(), renderer.get());
    ImGui_ImplSDLRenderer2_Init(renderer.get());

    auto io = &ImGui::GetIO();
    (void)*io;
    io->ConfigFlags |= config.flags;
    io->FontGlobalScale = config.font_scale;
    io->MouseDrawCursor = config.custom_mouse_cursor;

    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;

    io->IniFilename = config.ini_filename.c_str();
    core::Logger::debug("Loading layout from config file at: %s", io->IniFilename);
    /* _io->Fonts->Clear(); */
    /* _io->Fonts->AddFontFromFileTTF(_config.font_filename.c_str(), 14.0f); */
    /* _io->Fonts->Build(); */

    config.config_style();

    core::Logger::debug("ImGui Context successfully initialized");
}

void ImGuiContext::cleanup() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    core::Logger::debug("ImGui Context successfully cleaned up");
    m_cleaned_up = true;
}

}  // namespace contexts
}  // namespace piksy
