#include <imgui.h>

#include <components/frame_viewer.hpp>
#include <core/logger.hpp>
#include <cstdint>
#include <cstdio>

#include "rendering/frame.hpp"

namespace piksy {
namespace components {

FrameViewer::FrameViewer(core::State& state) : UIComponent(state) {}

void FrameViewer::update() {
    auto& animation = m_state.animation_state.get_current_animation();
    if (!m_state.animation_state.is_playing || animation.frames.empty()) return;

    m_state.animation_state.timer += m_state.delta_time;

    if (m_state.animation_state.timer >= m_state.animation_state.frame_duration) {
        m_state.animation_state.timer -= m_state.animation_state.frame_duration;
        m_state.animation_state.current_frame =
            (m_state.animation_state.current_frame + 1) % animation.frames.size();
    }
}

void FrameViewer::render() {
    auto texture = m_state.texture_sprite.texture();
    auto& animation = m_state.animation_state.get_current_animation();
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
            (m_state.animation_state.current_frame + 1) % animation.frames.size();
    }
    ImGui::SameLine(0, 4);
    if (ImGui::Button(ICON_FA_FAST_FORWARD, ImVec2(20, 20))) {
        m_state.animation_state.current_frame = animation.frames.size() - 1;
    }

    ImGui::End();

    if (ImGui::Begin("Frames")) {
        ImGui::Text("Frames: %zu", animation.frames.size());
        ImGui::BeginChild("##frames", ImVec2(0, 0), true);

        ImVec2 item_size(60, 60);
        float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

        for (size_t i = 0; i < animation.frames.size(); ++i) {
            const rendering::Frame& frame = animation.frames[i];

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
            if (i < animation.frames.size() - 1 && last_item_x2 + item_size.x < window_visible_x2) {
                ImGui::SameLine();
            }
        }

        ImGui::EndChild();
        ImGui::End();
    }

    render_frame_data();

    if (ImGui::Begin("Current Frame")) {
        if (animation.frames.empty()) {
            ImGui::Text("No frame to preview");
            ImGui::End();
        } else {
            const rendering::Frame& frame = animation.frames[m_state.animation_state.current_frame];
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

            render_background_grid(available_space);
            ImGui::Image((ImTextureID)(intptr_t)texture->get(), display_size, uv0, uv1);

            ImGui::End();
        }
    }
}

void FrameViewer::render_frame_data() const {
    auto& animation = m_state.animation_state.get_current_animation();
    ImGui::Begin("Frame Data");

    if (animation.frames.empty()) {
        ImGui::Text("No frames available. Select a frame to manage its data.");
        ImGui::End();
        return;
    }

    const size_t current_frame_index = m_state.animation_state.current_frame;
    // Frame must be non-const so we can modify `frame.data`
    rendering::Frame& current_frame = animation.frames[current_frame_index];

    ImGui::Text("Editing Frame: %zu", current_frame_index);
    ImGui::Separator();

    // Insert your custom data editor
    render_custom_data_editor(current_frame.data);

    ImGui::End();
}

/**
 * Renders an inline Key/Value data editor for `frame.data`.
 * - Adds a new key/value row
 * - Edits existing key and value
 * - Deletes pairs
 */
void FrameViewer::render_custom_data_editor(nlohmann::json& data_json) const {
    // We could put this in a collapsing header or an extra window
    // In your existing code, this is inside "ImGui::Begin(...)" and "ImGui::End(...)" scope.

    if (ImGui::CollapsingHeader("Custom Data (Key/Value)")) {
        // We'll show a table with columns Key | Value | Actions
        // optional: if you want advanced usage: ImGui::BeginTable("KV Table", 3)
        // For simplicity, let's do just a for-loop with inline items.

        //
        // 1) Render each existing (key, value) from the JSON
        //
        // Because editing keys in place can be tricky (keys are hashed in JSON),
        // we can store the old key, remove it if the key changes, and re-insert newKey.
        // Or we do a “two-phase commit”: show a child window, rename there, apply on "OK".
        // Below is a quick inline approach that re-writes the key if changed:
        //

        // We'll gather them first so we don't modify while iterating
        std::vector<std::string> keys_to_delete;
        std::vector<std::string> old_keys;  // store old keys
        std::vector<std::string> new_keys;  // store new keys

        // We might also collect new values as strings, if you want full control
        std::vector<std::string> new_values;

        // Step A: Collect all keys in a vector so we can iterate safely
        std::vector<std::string> all_keys;
        for (auto& kv : data_json.items()) {
            all_keys.push_back(kv.key());
        }

        // Step B: Render each row
        for (size_t i = 0; i < all_keys.size(); i++) {
            const std::string& key = all_keys[i];

            // Convert the current value to string for display
            // (If you store nested objects/arrays, you'll need a custom approach.)
            std::string value_str = data_json[key].dump();  // or `.dump()` to see JSON form

            ImGui::PushID(i);  // Unique ID for this row
            ImGui::Separator();

            //
            // Key editing
            //
            static char key_buffer[256];
            // Copy the current key into a buffer for editing
            // (only once, or every frame—your choice. For simplicity: every frame.)
            memset(key_buffer, 0, sizeof(key_buffer));
            strncpy(key_buffer, key.c_str(), sizeof(key_buffer) - 1);

            if (ImGui::InputText("Key", key_buffer, IM_ARRAYSIZE(key_buffer))) {
                // If edited, we store old->new so we can rename in JSON at the end
                old_keys.push_back(key);
                new_keys.push_back(std::string(key_buffer));
            }

            //
            // Value editing
            //
            static char value_buffer[512];
            memset(value_buffer, 0, sizeof(value_buffer));
            strncpy(value_buffer, value_str.c_str(), sizeof(value_buffer) - 1);

            if (ImGui::InputText("Value", value_buffer, IM_ARRAYSIZE(value_buffer))) {
                // We'll just store it to re-assign after the loop
                old_keys.push_back(key);  // we only need the old key
                new_keys.push_back(key);  // the key didn't change here
                new_values.push_back(value_buffer);
            }

            // Delete button
            ImGui::SameLine();
            if (ImGui::Button("Delete")) {
                keys_to_delete.push_back(key);
            }

            ImGui::PopID();  // end row
        }

        //
        // 2) Apply any modifications from the loop
        //
        // Because we might have multiple changes for the same row (key rename + value change),
        // we combine them carefully. We'll do something simple: For each old->new, apply them:
        //
        // Some “two-phase commit” care might be needed if you want perfect behavior,
        // but here is the simple approach:
        //
        for (auto& key : keys_to_delete) {
            data_json.erase(key);
        }

        // We'll pop from new_values in the order they were added
        size_t value_index = 0;

        // For each old->new, rename in JSON or assign new value
        // Because we used two separate triggers (key rename vs. value change),
        // we might have duplicates in old_keys/new_keys. We do a safe approach:
        for (size_t i = 0; i < old_keys.size(); i++) {
            const std::string& old_key = old_keys[i];
            const std::string& new_key = new_keys[i];

            // If the new_key is different from old_key, we rename:
            if (new_key != old_key) {
                // Save the old value to a temporary
                auto temp_val = data_json[old_key];
                // Erase old key
                data_json.erase(old_key);
                // Insert under new_key
                data_json[new_key] = temp_val;
            }

            // If we also have new_values pending for this row, let's apply them
            // We'll do a naive approach: if new_key == old_key, we assume it's a value change
            // Or we can do a separate array. We'll check if the row had a new value:
            if (value_index < new_values.size()) {
                // If new_key == old_key, that means it's purely a value update
                data_json[new_key] = nlohmann::json::parse(new_values[value_index]);
                value_index++;
            }
        }

        // Clear these arrays so they don't keep applying on next frame
        keys_to_delete.clear();
        old_keys.clear();
        new_keys.clear();
        new_values.clear();

        //
        // 3) Add a new K/V row
        //
        static char new_key_buf[256] = "";
        static char new_val_buf[512] = "";

        ImGui::Separator();
        ImGui::Text("Add New Entry:");
        ImGui::InputText("New Key", new_key_buf, IM_ARRAYSIZE(new_key_buf));
        ImGui::InputText("New Value", new_val_buf, IM_ARRAYSIZE(new_val_buf));
        ImGui::SameLine();
        if (ImGui::Button("Add##NewKV")) {
            if (strlen(new_key_buf) > 0) {
                // parse the new_val_buf as JSON if you want to store raw JSON
                // or store as string if you want it that way
                // e.g. data_json[new_key_buf] = new_val_buf;
                // or parse:
                try {
                    auto parsed_val = nlohmann::json::parse(new_val_buf);
                    data_json[new_key_buf] = parsed_val;
                } catch (...) {
                    // fallback if parsing fails, store as string
                    data_json[new_key_buf] = new_val_buf;
                }

                // Clear buffers for next addition
                new_key_buf[0] = '\0';
                new_val_buf[0] = '\0';
            }
        }
    }
}

void FrameViewer::render_background_grid(const ImVec2& size) const {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    const int grid_size = 10;
    const ImVec2& pos = ImGui::GetItemRectMin();

    for (float x = pos.x - size.y; x < pos.x + size.x; x += grid_size) {
        draw_list->AddLine(ImVec2(x, pos.y), ImVec2(x + size.y, pos.y + size.y),
                           IM_COL32(120, 120, 120, 80));
    }
}

void FrameViewer::adjust_pan_and_zoom_to_frame(int frame_index) {
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

void FrameViewer::delete_frame(size_t frame_index) {
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
