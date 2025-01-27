#include <imgui.h>

#include <components/animation_player.hpp>
#include <core/config.hpp>
#include <core/logger.hpp>
#include <core/state.hpp>

namespace piksy {
namespace components {

AnimationPlayer::AnimationPlayer(core::State& state) : UIComponent(state) {}

void AnimationPlayer::update() {}

void AnimationPlayer::render() {
    ImGui::Begin("Animation Player");
    ImGui::End();
}

}  // namespace components
}  // namespace piksy
