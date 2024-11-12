#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <components/animation_preview.hpp>
#include <core/logger.hpp>

#include "rendering/animation.hpp"

namespace piksy {
namespace components {

AnimationPreview::AnimationPreview(core::State& state) : UIComponent(state) {}

void AnimationPreview::update() {}

void AnimationPreview::render() {
    ImGui::Begin("Animation");

    if (m_state.texture_sprite.texture() == nullptr) {
        ImGui::Text("Please select a texture and select some frames to preview the animation.");
        ImGui::End();
        return;
    }

    auto& animation = m_state.animation_state.get_current_animation();
    const char* animation_label = m_state.animation_state.current_animation.c_str();

    render_animation_combo(animation_label);

    // Animation Name Editing and Deletion
    ImGui::Separator();
    ImGui::Text("Animation Options:");
    static char new_name[256] = "";
    if (ImGui::InputText("Rename Animation", new_name, IM_ARRAYSIZE(new_name),
                         ImGuiInputTextFlags_EnterReturnsTrue)) {
        if (new_name[0] != '\0') {  // Ensure non-empty name
            auto& animations = m_state.animation_state.animations;
            auto iter = animations.find(animation_label);
            if (iter != animations.end()) {
                animations[new_name] = std::move(iter->second);
                animations.erase(iter);
                m_state.animation_state.current_animation = new_name;
                strcpy(new_name, "");  // Clear the input field
            }
        }
    }

    if (ImGui::Button("Delete Animation")) {
        auto& animations = m_state.animation_state.animations;
        animations.erase(m_state.animation_state.current_animation);
        if (!animations.empty()) {
            m_state.animation_state.current_animation = animations.begin()->first;
        } else {
            m_state.animation_state.current_animation.clear();
        }
    }

    ImGui::Separator();
    render_frame_slider(animation);

    ImGui::End();
}

void AnimationPreview::render_animation_combo(const char* animation_label) {
    if (ImGui::BeginCombo("Select an animation", animation_label,
                          ImGuiComboFlags_PopupAlignLeft | ImGuiComboFlags_HeightSmall)) {
        for (const auto& [name, anim] : m_state.animation_state.animations) {
            bool is_selected = (name == m_state.animation_state.current_animation);

            if (ImGui::Selectable((std::string(ICON_FA_VIDEO_CAMERA) + " " + name).c_str(),
                                  is_selected)) {
                m_state.animation_state.current_animation = name;
                m_state.animation_state.selected_frames.clear();
            }

            if (is_selected) ImGui::SetItemDefaultFocus();
        }
        if (ImGui::Selectable(ICON_FA_PLUS " New Animation", false)) {
            std::string new_animation =
                "New Animation " + std::to_string(m_state.animation_state.animations.size());
            m_state.animation_state.animations[new_animation];
            m_state.animation_state.current_animation = new_animation;
            m_state.animation_state.selected_frames.clear();
        }
        ImGui::EndCombo();
    }
}

void AnimationPreview::render_frame_slider(const rendering::Animation& animation) {
    ImGui::BeginChild("##AnimationSlider");

    // Horizontal scroll for smooth navigation
    float wheel_H = ImGui::GetIO().MouseWheelH;
    if (wheel_H != 0.0f) {
        target_frame_offset += wheel_H;
    }

    // Smoothly lerp the current frame offset towards the target
    const float lerp_speed = 0.05f;
    current_frame_offset += (target_frame_offset - current_frame_offset) * lerp_speed;

    int frame_offset_index = static_cast<int>(std::round(current_frame_offset));
    int num_items = 7;

    ImVec2 item_size{125.f, 125.f};
    float gap = 40.0f;
    float total_width = (item_size.x * num_items) + (gap * (num_items - 1));
    float start_x = (ImGui::GetContentRegionAvail().x - total_width) / 2.0f;

    auto texture = m_state.texture_sprite.texture();
    ImTextureID tex_id = (ImTextureID)texture->get();
    ImVec2 window_pos = ImGui::GetCursorScreenPos();
    window_pos.y += ImGui::GetContentRegionAvail().y / 2 - item_size.y / 2;
    ImGui::SetCursorScreenPos(window_pos);

    int center_index = m_state.animation_state.current_frame + frame_offset_index;
    int half_num_items = num_items / 2;

    for (int i = 0; i < num_items; ++i) {
        int frame_index =
            (center_index + i - half_num_items + animation.frames.size()) % animation.frames.size();
        bool is_placeholder = frame_index >= animation.frames.size();

        ImVec2 pos = {window_pos.x + start_x + i * (item_size.x + gap), window_pos.y};
        ImGui::SetCursorScreenPos(pos);

        if (ImGui::InvisibleButton(("##frame" + std::to_string(i)).c_str(), item_size)) {
            if (!is_placeholder) {
                m_state.animation_state.current_frame = frame_index;
                adjust_pan_and_zoom_to_frame(frame_index);
            }
        }

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        int distance_from_center = std::abs(i - half_num_items);
        float scale_factor = 1.0f - 0.15f * distance_from_center;
        scale_factor = std::max(0.5f, scale_factor);

        ImVec2 scaled_item_size = {item_size.x * scale_factor, item_size.y * scale_factor};
        ImVec2 center_offset = {(item_size.x - scaled_item_size.x) / 2.0f,
                                (item_size.y - scaled_item_size.y) / 2.0f};
        ImVec2 scaled_pos = {pos.x + center_offset.x, pos.y + center_offset.y};

        ImU32 background_color = IM_COL32(30, 30, 30, 255 - (40 * distance_from_center));
        draw_list->AddRectFilled(
            scaled_pos, {scaled_pos.x + scaled_item_size.x, scaled_pos.y + scaled_item_size.y},
            background_color);

        if (is_placeholder) {
            ImU32 line_color = IM_COL32(255, 0, 0, 255);
            draw_list->AddLine(
                scaled_pos, {scaled_pos.x + scaled_item_size.x, scaled_pos.y + scaled_item_size.y},
                line_color, 2.0f);
            draw_list->AddLine({scaled_pos.x, scaled_pos.y + scaled_item_size.y},
                               {scaled_pos.x + scaled_item_size.x, scaled_pos.y}, line_color, 2.0f);
        } else {
            const rendering::Frame& frame = animation.frames[frame_index];
            ImVec2 uv0{static_cast<float>(frame.x) / texture->width(),
                       static_cast<float>(frame.y) / texture->height()};
            ImVec2 uv1{static_cast<float>(frame.x + frame.w) / texture->width(),
                       static_cast<float>(frame.y + frame.h) / texture->height()};

            float aspect_ratio = static_cast<float>(frame.w) / frame.h;
            ImVec2 image_size = scaled_item_size;
            if (aspect_ratio >= 1.0f) {
                image_size.y = scaled_item_size.x / aspect_ratio;
                image_size.x = scaled_item_size.x;
            } else {
                image_size.x = scaled_item_size.y * aspect_ratio;
                image_size.y = scaled_item_size.y;
            }

            ImVec2 image_offset = {(scaled_item_size.x - image_size.x) / 2.0f,
                                   (scaled_item_size.y - image_size.y) / 2.0f};
            draw_list->AddImage(tex_id,
                                {scaled_pos.x + image_offset.x, scaled_pos.y + image_offset.y},
                                {scaled_pos.x + image_offset.x + image_size.x,
                                 scaled_pos.y + image_offset.y + image_size.y},
                                uv0, uv1);

            ImVec2 text_size =
                ImGui::CalcTextSize(("Frame " + std::to_string(frame_index + 1)).c_str());
            ImVec2 text_pos = {pos.x + (item_size.x - text_size.x) / 2, pos.y + item_size.y + 5};
            ImGui::SetCursorScreenPos(text_pos);
            ImGui::Text("Frame %d", frame_index + 1);

            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("Frame %d Info:", frame_index + 1);
                ImGui::Text("Position: (%d, %d)", frame.x, frame.y);
                ImGui::Text("Size: %dx%d", frame.w, frame.h);
                ImGui::EndTooltip();
            }
        }

        if (i < num_items - 1) {
            ImGui::SameLine();
        }
    }

    ImGui::EndChild();
}

void AnimationPreview::adjust_pan_and_zoom_to_frame(int frame_index) {
    auto& animation = m_state.animation_state.get_current_animation();
    const rendering::Frame& frame = animation.frames[frame_index];
    float desired_scale = 4.0f;
    m_state.zoom_state.target_scale = desired_scale;

    float frame_center_x = frame.x + frame.w / 2.0f;
    float frame_center_y = frame.y + frame.h / 2.0f;

    ImVec2 viewport_size = m_state.viewport_state.size;

    float pan_offset_x = (viewport_size.x / 2.0f) / desired_scale - frame_center_x;
    float pan_offset_y = (viewport_size.y / 2.0f) / desired_scale - frame_center_y;

    m_state.pan_state.target_offset.x = pan_offset_x;
    m_state.pan_state.target_offset.y = pan_offset_y;
}

void AnimationPreview::delete_frame(size_t frame_index) {
    if (m_state.animation_state.selected_frames.count(frame_index)) {
        m_state.animation_state.selected_frames.erase(frame_index);
    }

    auto& animation = m_state.animation_state.get_current_animation();
    if (frame_index < animation.frames.size()) {
        animation.frames.erase(animation.frames.begin() + frame_index);
    }
}

}  // namespace components
}  // namespace piksy
