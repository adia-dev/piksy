#include <SDL2/SDL_image.h>
#include <SDL_rect.h>
#include <SDL_render.h>
#include <imgui.h>

#include <components/animation_preview.hpp>
#include <core/logger.hpp>
#include <core/state.hpp>

namespace piksy {
namespace components {

void AnimationPreview::update(core::State& state) {}

void AnimationPreview::render(core::State& state) {
    ImGui::Begin("Animation Preview");

    ImGui::End();
}

}  // namespace components
}  // namespace piksy
