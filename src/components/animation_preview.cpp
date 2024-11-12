#include <imgui.h>

#include <components/animation_preview.hpp>
#include <core/logger.hpp>
#include <cstdint>
#include <cstdio>

#include "rendering/frame.hpp"

namespace piksy {
namespace components {

AnimationPreview::AnimationPreview(core::State& state) : UIComponent(state) {}

void AnimationPreview::update() {
    if (!m_state.animation_state.is_playing || m_state.frames.empty()) return;

    m_state.animation_state.timer += m_state.delta_time;

    if (m_state.animation_state.timer >= m_state.animation_state.frame_duration) {
        m_state.animation_state.timer -= m_state.animation_state.frame_duration;
        m_state.animation_state.current_frame =
            (m_state.animation_state.current_frame + 1) % m_state.frames.size();
    }
}

void AnimationPreview::render() {
    auto texture = m_state.texture_sprite.texture();
    if (!texture) {
        ImGui::Text("Please select a texture and select some frames to visualize the preview.");
        return;
    }

    ImGui::Begin("Animation Controls", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse);

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::DragFloat("##FPS", &m_state.animation_state.fps, 1.0f, 1.0f, 60.0f, "%.0f FPS")) {
        m_state.animation_state.frame_duration = 1.0f / m_state.animation_state.fps;
    }

    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 100) / 2);

    if (ImGui::Button(ICON_FA_BACKWARD, ImVec2(20, 20))) {
        m_state.animation_state.current_frame = 0;
    }
    ImGui::SameLine(0, 4);
    if (ImGui::Button(ICON_FA_STEP_BACKWARD, ImVec2(20, 20))) {
        if (--m_state.animation_state.current_frame < 0) m_state.animation_state.current_frame = 0;
    }
    ImGui::SameLine(0, 4);
    if (ImGui::Button(m_state.animation_state.is_playing ? ICON_FA_PAUSE : ICON_FA_PLAY,
                      ImVec2(20, 20))) {
        m_state.animation_state.is_playing = !m_state.animation_state.is_playing;
    }
    ImGui::SameLine(0, 4);
    if (ImGui::Button(ICON_FA_STEP_FORWARD, ImVec2(20, 20))) {
        m_state.animation_state.current_frame =
            (m_state.animation_state.current_frame + 1) % m_state.frames.size();
    }
    ImGui::SameLine(0, 4);
    if (ImGui::Button(ICON_FA_FAST_FORWARD, ImVec2(20, 20))) {
        m_state.animation_state.current_frame = m_state.frames.size() - 1;
    }

    ImGui::End();

    if (ImGui::Begin("Frames")) {
        ImGui::Text("Frames: %zu", m_state.frames.size());
        ImGui::BeginChild("##frames", ImVec2(0, 0), true);

        ImVec2 item_size(60, 60);
        float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

        for (size_t i = 0; i < m_state.frames.size(); ++i) {
            const rendering::Frame& frame = m_state.frames[i];

            ImVec2 uv0 = {static_cast<float>(frame.x) / texture->width(),
                          static_cast<float>(frame.y) / texture->height()};
            ImVec2 uv1 = {static_cast<float>(frame.x + frame.w) / texture->width(),
                          static_cast<float>(frame.y + frame.h) / texture->height()};

            float aspect_ratio = static_cast<float>(frame.w) / frame.h;
            ImVec2 display_size = item_size;
            if (aspect_ratio >= 1.0f) {
                display_size.y = item_size.x / aspect_ratio;
                if (display_size.y > item_size.y) {
                    display_size.y = item_size.y;
                    display_size.x = item_size.y * aspect_ratio;
                }
            } else {
                display_size.x = item_size.y * aspect_ratio;
                if (display_size.x > item_size.x) {
                    display_size.x = item_size.x;
                    display_size.y = item_size.x / aspect_ratio;
                }
            }

            ImVec2 center_offset = {(item_size.x - display_size.x) / 2.0f,
                                    (item_size.y - display_size.y) / 2.0f};

            ImGui::PushID(static_cast<int>(i));

            ImU32 bg_color = (i == m_state.animation_state.current_frame)
                                 ? IM_COL32(100, 200, 100, 255)
                                 : IM_COL32(80, 80, 80, 200);

            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
            draw_list->AddRectFilled(cursor_pos,
                                     {cursor_pos.x + item_size.x, cursor_pos.y + item_size.y},
                                     bg_color, 4.0f);

            if (ImGui::InvisibleButton((std::string("##") + std::to_string(i)).c_str(),
                                       item_size)) {
                m_state.animation_state.current_frame = i;
                adjust_pan_and_zoom_to_frame(i);
            }

            draw_list->AddImage((ImTextureID)(intptr_t)texture->get(),
                                {cursor_pos.x + center_offset.x, cursor_pos.y + center_offset.y},
                                {cursor_pos.x + center_offset.x + display_size.x,
                                 cursor_pos.y + center_offset.y + display_size.y},
                                uv0, uv1);

            ImGui::PopID();

            float last_item_x2 = ImGui::GetItemRectMax().x;
            if (i < m_state.frames.size() - 1 && last_item_x2 + item_size.x < window_visible_x2) {
                ImGui::SameLine();
            }
        }

        ImGui::EndChild();
        ImGui::End();
    }

    if (ImGui::Begin("Current Frame")) {
        if (m_state.frames.empty()) {
            ImGui::Text("No frame to preview");
            ImGui::End();
        } else {
            const rendering::Frame& frame = m_state.frames[m_state.animation_state.current_frame];
            ImVec2 uv0 = {static_cast<float>(frame.x) / texture->width(),
                          static_cast<float>(frame.y) / texture->height()};
            ImVec2 uv1 = {static_cast<float>(frame.x + frame.w) / texture->width(),
                          static_cast<float>(frame.y + frame.h) / texture->height()};

            ImVec2 available_space = ImGui::GetContentRegionAvail();
            float aspect_ratio = static_cast<float>(frame.w) / frame.h;
            ImVec2 display_size = available_space;

            if (aspect_ratio >= 1.0f) {
                display_size.y = available_space.x / aspect_ratio;
                if (display_size.y > available_space.y) {
                    display_size.y = available_space.y;
                    display_size.x = available_space.y * aspect_ratio;
                }
            } else {
                display_size.x = available_space.y * aspect_ratio;
                if (display_size.x > available_space.x) {
                    display_size.x = available_space.x;
                    display_size.y = available_space.x / aspect_ratio;
                }
            }

            ImVec2 center_offset = {(available_space.x - display_size.x) / 2.0f,
                                    available_space.y - display_size.y};
            ImGui::SetCursorPos({ImGui::GetCursorPos().x + center_offset.x,
                                 ImGui::GetCursorPos().y + center_offset.y});

            draw_background_grid(ImGui::GetItemRectMin(), available_space);
            ImGui::Image((ImTextureID)(intptr_t)texture->get(), display_size, uv0, uv1);

            ImGui::End();
        }
    }
}

void AnimationPreview::draw_background_grid(const ImVec2& pos, const ImVec2& size) const {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    const int grid_size = 10;

    for (float x = pos.x - size.y; x < pos.x + size.x; x += grid_size) {
        draw_list->AddLine(ImVec2(x, pos.y), ImVec2(x + size.y, pos.y + size.y),
                           IM_COL32(120, 120, 120, 80));
    }
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
