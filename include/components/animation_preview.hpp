#pragma once

#include <imgui.h>

#include <components/ui_component.hpp>
#include <core/logger.hpp>
#include <rendering/renderer.hpp>

namespace piksy {
namespace components {

class AnimationPreview : public UIComponent {
   public:
    void update(core::State& state) override;
    void render(core::State& state) override;

   private:
    float _last_frame_time = 0.0f;
};

}  // namespace components
}  // namespace piksy
