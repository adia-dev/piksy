#pragma once

#include <layers/editor_layer.hpp>

#include "components/console.hpp"
#include "core/logger.hpp"
#include "imgui.h"
#include "managers/resource_manager.hpp"
#include "rendering/renderer.hpp"

namespace piksy {
namespace layers {

EditorLayer::EditorLayer(rendering::Renderer& renderer, managers::ResourceManager& resource_manager,
                         core::State& state)
    : m_renderer(renderer), m_resource_manager(resource_manager), Layer(state, "EditorLayer") {}

void EditorLayer::on_attach() {
    m_viewport = std::make_unique<components::Viewport>(m_state, m_renderer, m_resource_manager);
    m_console = std::make_unique<components::Console>(m_state);
    m_project = std::make_unique<components::Project>(m_state, m_resource_manager);
    m_frame_viewer = std::make_unique<components::FrameViewer>(m_state);
    m_animation_preview = std::make_unique<components::AnimationPreview>(m_state);

    core::Logger::debug("Attached the EditorLayer");
}

void EditorLayer::on_detach() { core::Logger::debug("Detached the EditorLayer"); }

void EditorLayer::on_update(float dt) {
    m_viewport->update();
    m_console->update();
    m_project->update();
    m_frame_viewer->update();
    m_animation_preview->update();
}

void EditorLayer::on_render() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Editor", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleVar();

    m_viewport->render();
    m_console->render();
    m_project->render();
    m_frame_viewer->render();
    m_animation_preview->render();

    ImGui::End();
}

void EditorLayer::on_event(SDL_Event& event, bool& event_handled) {
    if (event.type == SDL_KEYDOWN) {
        core::Logger::info("Pressed key: %d", event.key.keysym.sym);
        event_handled = true;
    }
}

}  // namespace layers
}  // namespace piksy
