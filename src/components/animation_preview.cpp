#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <components/animation_preview.hpp>
#include <core/logger.hpp>

namespace piksy {
namespace components {

AnimationPreview::AnimationPreview(core::State& state) : UIComponent(state) {}

void AnimationPreview::update() {}

void AnimationPreview::render() {
    ImGui::Begin("Animation");

    ImGui::BeginChild("##AnimationSlider");

    // Adjust target frame offset based on horizontal scroll input
    float wheel_H = ImGui::GetIO().MouseWheelH;
    if (wheel_H != 0.0f) {
        target_frame_offset += wheel_H;  // Adjust the speed of scroll here if needed
    }

    // Smoothly lerp the current frame offset towards the target
    const float lerp_speed = 0.05f;
    current_frame_offset += (target_frame_offset - current_frame_offset) * lerp_speed;

    // Wrap `current_frame_offset` to an integer frame index
    int frame_offset_index = static_cast<int>(std::round(current_frame_offset));
    int num_items = 7;  // Display up to 7 items, including placeholders if needed

    // Calculate centered layout position
    ImVec2 item_size{125.f, 125.f};
    float gap = 40.0f;
    float total_width = (item_size.x * num_items) + (gap * (num_items - 1));
    float start_x = (ImGui::GetContentRegionAvail().x - total_width) / 2.0f;

    auto texture = m_state.texture_sprite.texture();
    ImTextureID tex_id = (ImTextureID)texture->get();
    ImVec2 window_pos = ImGui::GetCursorScreenPos();
    window_pos.y += ImGui::GetContentRegionAvail().y / 2 - item_size.y / 2;
    ImGui::SetCursorScreenPos(window_pos);

    // Calculate center frame index and offset other frames around it
    int center_index = m_state.animation_state.current_frame + frame_offset_index;
    int half_num_items = num_items / 2;

    for (int i = 0; i < num_items; ++i) {
        int frame_index =
            (center_index + i - half_num_items + m_state.frames.size()) % m_state.frames.size();
        bool is_placeholder = frame_index >= m_state.frames.size();

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
            const rendering::Frame& frame = m_state.frames[frame_index];
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
    ImGui::End();
}

void AnimationPreview::adjust_pan_and_zoom_to_frame(int frame_index) {
    const rendering::Frame& frame = m_state.frames[frame_index];
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
    if (m_state.selected_frames.count(frame_index)) {
        m_state.selected_frames.erase(frame_index);
    }
    if (frame_index < m_state.frames.size()) {
        m_state.frames.erase(m_state.frames.begin() + frame_index);
    }
}

}  // namespace components
}  // namespace piksy
