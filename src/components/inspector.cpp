#include <imgui.h>

#include <components/inspector.hpp>

namespace piksy {
namespace components {

void Inspector::render(core::State& state) {
    ImGui::Begin("Inspector");
    render_sprite(state);

    ImGui::End();
}

void Inspector::render_sprite(core::State& state) {
    if (ImGui::TreeNode("Sprite")) {
        auto& sprite = state.texture_sprite;
        static int position[2]{sprite.x(), sprite.y()};

        ImGui::SeparatorText("Goku SSJG");
        ImGui::SameLine();
        help_marker("Enable keyboard controls.");

        if (ImGui::SliderInt2("Position", position, -3000, 3000)) {
            sprite.set_position(position[0], position[1]);
        }

        ImGui::TreePop();
    }
}

}  // namespace components
}  // namespace piksy
