#pragma once

#include <SDL_events.h>

#include <components/animation_preview.hpp>
#include <components/frame_viewer.hpp>
#include <components/project.hpp>
#include <components/viewport.hpp>
#include <layers/layer.hpp>
#include <managers/resource_manager.hpp>
#include <memory>
#include <rendering/renderer.hpp>

#include "components/console.hpp"

namespace piksy {
namespace layers {
class EditorLayer : public Layer {
   public:
    EditorLayer(rendering::Renderer& renderer, managers::ResourceManager& resource_manager,
                core::State& state);

    void on_attach() override;
    void on_detach() override;
    void on_update(float dt) override;
    void on_render() override;
    void on_event(SDL_Event& event, bool& event_handled) override;

   private:
    rendering::Renderer& m_renderer;
    managers::ResourceManager& m_resource_manager;

    std::unique_ptr<components::Viewport> m_viewport;
    std::unique_ptr<components::Console> m_console;
    std::unique_ptr<components::Project> m_project;
    std::unique_ptr<components::FrameViewer> m_frame_viewer;
    std::unique_ptr<components::AnimationPreview> m_animation_preview;
};
}  // namespace layers
}  // namespace piksy
