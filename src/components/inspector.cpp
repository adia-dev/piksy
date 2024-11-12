#include <imgui.h>

#include <components/inspector.hpp>
#include <cstring>
#include <stdexcept>

#include "components/ui_component.hpp"
#include "core/logger.hpp"
#include "core/state.hpp"

namespace piksy {
namespace components {
Inspector::Inspector(core::State& state, rendering::Renderer& renderer)
    : UIComponent(state), m_renderer(renderer) {}

void Inspector::update() {}

void Inspector::render() {
    ImGui::Begin("Inspector");

    ImGui::Text("Inspector Settings");
    help_marker(
        "Use the inspector to modify sprite properties and textures. Hover over (?) icons for "
        "help.");

    ImGui::Separator();

    ImGui::Text("Replacement Color");
    ImGui::SameLine();
    help_marker("This color is used to replace specific colors in the texture.");
    ImGui::ColorEdit4("##Replacement Color", reinterpret_cast<float*>(&m_state.replacement_color),
                      ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel |
                          ImGuiColorEditFlags_AlphaPreview);

    ImGui::Separator();

    ImGui::Text("Viewport");
    // TODO: rewrite this help marker
    help_marker("This color is used to replace specific colors in the texture.");
    ImGui::SliderInt("##Viewport Grid Cell Size", &m_state.viewport_state.grid_cell_size, 0, 1000);

    ImGui::Separator();

    render_sprite_properties();

    ImGui::End();
}

void Inspector::render_sprite_properties() {
    if (ImGui::CollapsingHeader("Sprite Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto& sprite = m_state.texture_sprite;

        int position[2]{sprite.x(), sprite.y()};
        ImGui::Text("Position");
        help_marker("Adjust the X and Y position of the sprite on the screen.");
        if (ImGui::DragInt2("##Position", position, 1.0f)) {
            sprite.set_position(position[0], position[1]);
        }

        ImGui::Text("Scale");
        help_marker("Adjust the width and height of the sprite.");
        float scale[2]{static_cast<float>(sprite.width()), static_cast<float>(sprite.height())};
        if (ImGui::DragFloat2("##Scale", scale, 0.1f, 1.0f, 500.0f)) {
            sprite.set_size(static_cast<int>(scale[0]), static_cast<int>(scale[1]));
        }

        ImGui::Text("Frame Rect");
        help_marker(
            "Specify the sub-rectangle of the texture to display (useful for spritesheets).");
        int frame_rect[4]{sprite.frame_rect().x, sprite.frame_rect().y, sprite.frame_rect().w,
                          sprite.frame_rect().h};
        if (ImGui::InputInt4("##Frame Rect", frame_rect)) {
            sprite.set_frame_rect({frame_rect[0], frame_rect[1], frame_rect[2], frame_rect[3]});
        }

        ImGui::Separator();
        render_texture_properties(sprite.texture());
    }
}

void Inspector::render_texture_properties(std::shared_ptr<rendering::Texture2D> texture) {
    if (texture && ImGui::CollapsingHeader("Texture Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
        static bool buffer_initialized = false;
        static char _buffer[4096]{0};
        if (!buffer_initialized) {
            memcpy(_buffer, texture->path().c_str(), texture->path().length());
            buffer_initialized = true;
        }
        if (ImGui::InputText("Texture Path", _buffer, 4096)) {
            texture->set_path(_buffer);
        }
        ImGui::Separator();
        ImGui::Text("Texture Dimensions");
        help_marker("Displays the width and height of the texture.");

        ImGui::Text("Width: %d", texture->width());
        ImGui::SameLine();
        ImGui::Text("Height: %d", texture->height());

        ImGui::Separator();
        if (ImGui::Button("Reload Texture")) {
            try {
                texture->reload(m_renderer.get());
            } catch (const std::runtime_error& ex) {
                core::Logger::error("Failed to reload the texture: %s", ex.what());
            }
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
