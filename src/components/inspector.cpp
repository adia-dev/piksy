#include <imgui.h>

#include <components/inspector.hpp>

namespace piksy {
namespace components {

void Inspector::update() {}

void Inspector::render(core::State& state) {
    ImGui::Begin("Inspector");
    render_sprite_properties(state);
    ImGui::End();
}

void Inspector::render_sprite_properties(core::State& state) {
    if (ImGui::CollapsingHeader("Sprite Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto& sprite = state.texture_sprite;
        // Position
        int position[2]{sprite.x(), sprite.y()};
        if (ImGui::DragInt2("Position", position, 1.0f)) {
            sprite.set_position(position[0], position[1]);
        }

        // Scale
        float scale[2]{static_cast<float>(sprite.width()), static_cast<float>(sprite.height())};
        if (ImGui::DragFloat2("Scale", scale, 0.01f, 0.1f, 10.0f)) {
            sprite.set_size(scale[0], scale[1]);
        }
    }
}

void Inspector::help_marker(const std::string& desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

}  // namespace components
}  // namespace piksy
