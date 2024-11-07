#include <imgui.h>

#include <components/inspector.hpp>
#include <cstring>
#include <stdexcept>

#include "core/logger.hpp"
#include "core/state.hpp"

namespace piksy {
namespace components {

void Inspector::update(core::State& state) {}

void Inspector::render(core::State& state) {
    ImGui::Begin("Inspector");

    // Add a header with help text
    ImGui::Text("Inspector Settings");
    help_marker(
        "Use the inspector to modify sprite properties and textures. Hover over (?) icons for "
        "help.");

    // Divider
    ImGui::Separator();

    // Replacement Color Picker with help marker
    ImGui::Text("Replacement Color");
    help_marker("This color is used to replace specific colors in the texture.");
    ImGui::ColorEdit4("##Replacement Color", state.replacement_color);

    // Divider for viewport controls
    ImGui::Separator();

    ImGui::Text("Viewport");
    // TODO: rewrite this help marker
    help_marker("This color is used to replace specific colors in the texture.");
    ImGui::SliderInt("##Viewport Grid Cell Size", &state.viewport_grid_cell_size, 0, 1000);

    // Divider
    ImGui::Separator();

    // Render sprite properties
    render_sprite_properties(state);

    ImGui::End();
}

void Inspector::render_sprite_properties(core::State& state) {
    if (ImGui::CollapsingHeader("Sprite Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto& sprite = state.texture_sprite;

        // Position Controls with help marker
        int position[2]{sprite.x(), sprite.y()};
        ImGui::Text("Position");
        help_marker("Adjust the X and Y position of the sprite on the screen.");
        if (ImGui::DragInt2("##Position", position, 1.0f)) {
            sprite.set_position(position[0], position[1]);
        }

        // Scale Controls with help marker
        ImGui::Text("Scale");
        help_marker("Adjust the width and height of the sprite.");
        float scale[2]{static_cast<float>(sprite.width()), static_cast<float>(sprite.height())};
        if (ImGui::DragFloat2("##Scale", scale, 0.1f, 1.0f, 500.0f)) {
            sprite.set_size(static_cast<int>(scale[0]), static_cast<int>(scale[1]));
        }

        // Frame Rect Controls (for animations or sub-textures)
        ImGui::Text("Frame Rect");
        help_marker(
            "Specify the sub-rectangle of the texture to display (useful for spritesheets).");
        int frame_rect[4]{sprite.frame_rect().x, sprite.frame_rect().y, sprite.frame_rect().w,
                          sprite.frame_rect().h};
        if (ImGui::InputInt4("##Frame Rect", frame_rect)) {
            sprite.set_frame_rect({frame_rect[0], frame_rect[1], frame_rect[2], frame_rect[3]});
        }

        // Divider for texture-specific controls
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
        // Texture Width and Height with help marker
        ImGui::Text("Texture Dimensions");
        help_marker("Displays the width and height of the texture.");

        ImGui::Text("Width: %d", texture->width());
        ImGui::SameLine();
        ImGui::Text("Height: %d", texture->height());

        // Reload Texture Button
        ImGui::Separator();
        if (ImGui::Button("Reload Texture")) {
            // Reload the texture, assuming a renderer is accessible in your state
            try {
                texture->reload(_renderer.get());
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

Inspector::Inspector(rendering::Renderer& renderer) : _renderer(renderer) {}
}  // namespace components
}  // namespace piksy
