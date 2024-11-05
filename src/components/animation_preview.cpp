#include <components/animation_preview.hpp>
#include <core/state.hpp>

namespace piksy {
namespace components {

void AnimationPreview::update() {}
void AnimationPreview::render(core::State& state) {
    ImGui::Begin("Animation Preview");
    ImGui::End();
}

}  // namespace components
}  // namespace piksy
