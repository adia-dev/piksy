#include <layers/editor_layer.hpp>
#include <memory>

#include "components/animation_player.hpp"
#include "components/console.hpp"
#include "core/logger.hpp"
#include "managers/animation_manager.hpp"
#include "managers/resource_manager.hpp"
#include "rendering/renderer.hpp"

namespace piksy {
namespace layers {

EditorLayer::EditorLayer(rendering::Renderer& renderer, core::State& state,
                         managers::ResourceManager& resource_manager,
                         managers::AnimationManager& animation_manager)
    : Layer(state, "EditorLayer"),
      m_renderer(renderer),
      m_resource_manager(resource_manager),
      m_animation_manager(animation_manager) {}

void EditorLayer::on_attach() {
    m_viewport = std::make_unique<components::Viewport>(m_state, m_renderer, m_resource_manager,
                                                        m_animation_manager);
    m_console = std::make_unique<components::Console>(m_state);
    m_animation_player = std::make_unique<components::AnimationPlayer>(m_state);
    m_project = std::make_unique<components::Project>(m_state, m_resource_manager);

    core::Logger::debug("Attached the EditorLayer");
}

void EditorLayer::on_detach() { core::Logger::debug("Detached the EditorLayer"); }

void EditorLayer::on_update(float dt) {
    m_viewport->update();
    m_console->update();
    m_animation_player->update();
    m_project->update();
}

void EditorLayer::on_render() {
    m_viewport->render();
    m_console->render();
    m_animation_player->render();
    m_project->render();
}

void EditorLayer::on_event(SDL_Event& event, bool& event_handled) {
    if (event.type == SDL_KEYDOWN) {
        core::Logger::info("Pressed key: %d", event.key.keysym.sym);
        event_handled = true;
    }
}

}  // namespace layers
}  // namespace piksy
