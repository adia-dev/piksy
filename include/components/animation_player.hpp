#pragma once

#include <imgui.h>

#include <components/ui_component.hpp>
#include <core/logger.hpp>
#include <core/state.hpp>
#include <vector>

namespace piksy {
namespace components {

class AnimationPlayer : public UIComponent {
   public:
    explicit AnimationPlayer(core::State& state);
    void update() override;
    void render() override;

   private:
};

}  // namespace components
}  // namespace piksy
