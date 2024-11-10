#include <SDL2/SDL_image.h>
#include <SDL_rect.h>
#include <SDL_render.h>
#include <imgui.h>

#include <components/animation_preview.hpp>
#include <core/logger.hpp>
#include <core/state.hpp>

namespace piksy {
namespace components {
AnimationPreview::AnimationPreview(core::State& state) : UIComponent(state) {}

void AnimationPreview::update() {}

void AnimationPreview::render() {
    ImGui::Begin("Animation Preview");

    ImGui::End();
}

}  // namespace components
}  // namespace piksy
