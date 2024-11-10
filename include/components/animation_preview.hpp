#pragma once

#include <imgui.h>

#include <components/ui_component.hpp>
#include <core/logger.hpp>
#include <rendering/renderer.hpp>

#include "core/state.hpp"

namespace piksy {
namespace components {

class AnimationPreview : public UIComponent {
   public:
    explicit AnimationPreview(core::State& state);
    void update() override;
    void render() override;

   private:
    float _last_frame_time = 0.0f;
};

}  // namespace components
}  // namespace piksy
