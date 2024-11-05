#pragma once

#include <imgui.h>

#include <components/ui_component.hpp>
#include <core/logger.hpp>

namespace piksy {
namespace components {

class AnimationPreview : public UIComponent {
   public:
    void update() override;
    void render(core::State& state) override;

   private:
};

}  // namespace components
}  // namespace piksy
