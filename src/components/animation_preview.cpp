
#include <imgui.h>

#include <components/animation_preview.hpp>
#include <core/logger.hpp>

namespace piksy {
namespace components {

AnimationPreview::AnimationPreview(core::State& state) : UIComponent(state) {}

void AnimationPreview::update() {
    // Update logic can be added here if needed
}

void AnimationPreview::render() {
    ImGui::Begin("Animation Preview");
    auto texture = _state.texture_sprite.texture();

    if (texture == nullptr) {
        ImGui::Text("Please select a texture and select some frames to visualize the preview.");
        ImGui::End();
        return;
    }

    render_controls();

    if (_state.frames.empty()) {
        ImGui::Text("Please select an area of your texture to extract frames for animation.");
        ImGui::End();
        return;
    }

    render_frames();

    ImGui::End();
}

void AnimationPreview::render_controls() {
    ImGui::Text("Animation Controls");
    ImGui::Separator();

    if (ImGui::Button(_state.animation_state.is_playing ? "Pause" : "Play")) {
        _state.animation_state.is_playing = !_state.animation_state.is_playing;
    }

    ImGui::SameLine();

    float slider_width = 128.0f;

    ImGui::SetNextItemWidth(slider_width);
    ImGui::SliderFloat("FPS", &_state.animation_state.fps, 1.0f, 60.0f, "%.1f FPS");

    ImGui::SameLine();

    ImGui::SetNextItemWidth(slider_width);
    ImGui::SliderFloat("Thumbnail Size", &_frame_display_size, 0.0f, 200.0f, "%.1f");

    ImGui::Separator();
}

void AnimationPreview::render_frames() {
    if (_frame_display_size > 32.0f) {
        render_frame_thumbnails();
    } else {
        render_frame_list();
    }
}

void AnimationPreview::adjust_pan_and_zoom_to_frame(int frame_index) {
    const SDL_Rect& frame = _state.frames[frame_index];

    float desired_scale = 4.0f;
    _state.zoom_state.target_scale = desired_scale;

    float frame_center_x = frame.x + frame.w / 2.0f;
    float frame_center_y = frame.y + frame.h / 2.0f;

    ImVec2 viewport_size = _state.viewport_state.size;

    float pan_offset_x = (viewport_size.x / 2.0f) / desired_scale - frame_center_x;
    float pan_offset_y = (viewport_size.y / 2.0f) / desired_scale - frame_center_y;

    _state.pan_state.target_offset.x = pan_offset_x;
    _state.pan_state.target_offset.y = pan_offset_y;
}

void AnimationPreview::render_frame_thumbnails() {
    auto texture = _state.texture_sprite.texture();

    ImGui::BeginChild("FrameScroller", ImVec2(0, _frame_display_size + 50), false,
                      ImGuiWindowFlags_HorizontalScrollbar);

    const float frame_padding = 5.0f;
    ImGuiStyle& style = ImGui::GetStyle();
    float old_item_spacing_x = style.ItemSpacing.x;
    style.ItemSpacing.x = frame_padding;

    int frame_count = static_cast<int>(_state.frames.size());
    for (int i = 0; i < frame_count; ++i) {
        SDL_Rect& frame = _state.frames[i];

        float aspect_ratio = static_cast<float>(frame.w) / frame.h;
        ImVec2 button_size(_frame_display_size - i, _frame_display_size - i);

        ImVec2 display_size;
        if (aspect_ratio >= 1.0f) {
            display_size.x = _frame_display_size;
            display_size.y = _frame_display_size / aspect_ratio;
        } else {
            display_size.x = _frame_display_size * aspect_ratio;
            display_size.y = _frame_display_size;
        }

        ImGui::BeginGroup();

        ImVec2 cursor_pos = ImGui::GetCursorPos();
        ImVec2 image_pos = ImGui::GetCursorScreenPos();
        image_pos.x += (button_size.x - display_size.x) * 0.5f;
        image_pos.y += (button_size.y - display_size.y) * 0.5f;

        ImGui::PushID(i);
        if (ImGui::InvisibleButton("frame_button", button_size)) {
            _state.animation_state.current_frame = i;
            _state.animation_state.is_playing = false;
            adjust_pan_and_zoom_to_frame(i);
        }

        if (ImGui::BeginPopupContextItem("ItemContextMenu")) {
            if (ImGui::MenuItem("Delete")) {
                delete_frame(i);
            }
            ImGui::EndPopup();
        }

        if (ImGui::BeginDragDropSource()) {
            ImGui::SetDragDropPayload("FRAME_PAYLOAD", &i, sizeof(int));
            ImGui::Text("Move Frame %d", i + 1);

            ImTextureID tex_id = (ImTextureID)(intptr_t)texture->get();

            ImVec2 uv0(static_cast<float>(frame.x) / texture->width(),
                       static_cast<float>(frame.y) / texture->height());
            ImVec2 uv1(static_cast<float>(frame.x + frame.w) / texture->width(),
                       static_cast<float>(frame.y + frame.h) / texture->height());

            ImGui::Image(tex_id, {100, 100}, uv0, uv1);

            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FRAME_PAYLOAD")) {
                int src_index = *(const int*)payload->Data;
                if (src_index != i) {
                    std::swap(_state.frames[src_index], _state.frames[i]);
                }
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::PopID();

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImTextureID tex_id = (ImTextureID)(intptr_t)texture->get();

        ImVec2 uv0(static_cast<float>(frame.x) / texture->width(),
                   static_cast<float>(frame.y) / texture->height());
        ImVec2 uv1(static_cast<float>(frame.x + frame.w) / texture->width(),
                   static_cast<float>(frame.y + frame.h) / texture->height());

        draw_list->AddImage(tex_id, image_pos,
                            ImVec2(image_pos.x + display_size.x, image_pos.y + display_size.y), uv0,
                            uv1);

        if (_state.animation_state.current_frame == i) {
            draw_list->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
                               IM_COL32(0, 255, 0, 255), 0.0f, 0, 2.0f);
        }

        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Frame %d", i + 1);
            ImGui::Text("Size: %dx%d", frame.w, frame.h);
            ImGui::EndTooltip();
        }

        ImVec2 label_pos = ImVec2(cursor_pos.x, cursor_pos.y + button_size.y + 3.0f);
        ImGui::SetCursorPos(label_pos);
        ImGui::TextWrapped("Frame %d", i + 1);

        ImGui::EndGroup();

        if (i < frame_count - 1) {
            ImGui::SameLine();
        }
    }

    style.ItemSpacing.x = old_item_spacing_x;

    ImGui::EndChild();
}

void AnimationPreview::render_frame_list() {
    if (ImGui::BeginTable("FrameListTable", 2, ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("Frame List", ImGuiTableColumnFlags_WidthStretch, 0.75f);
        ImGui::TableSetupColumn("Frame Preview", ImGuiTableColumnFlags_WidthStretch, 0.25f);
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        ImGui::BeginChild("FrameList", ImVec2(0, 0), true);

        int frame_count = static_cast<int>(_state.frames.size());
        for (int i = 0; i < frame_count; ++i) {
            ImGui::PushID(i);
            if (ImGui::Selectable(("Frame " + std::to_string(i + 1)).c_str(),
                                  _state.animation_state.current_frame == i)) {
                _state.animation_state.current_frame = i;
                _state.animation_state.is_playing = false;
                adjust_pan_and_zoom_to_frame(i);
            }

            if (ImGui::BeginPopupContextItem("ItemContextMenu")) {
                if (ImGui::MenuItem("Delete")) {
                    delete_frame(i);
                }
                ImGui::EndPopup();
            }

            if (ImGui::BeginDragDropSource()) {
                ImGui::SetDragDropPayload("FRAME_PAYLOAD", &i, sizeof(int));
                ImGui::Text("Move Frame %d", i + 1);
                ImGui::EndDragDropSource();
            }

            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FRAME_PAYLOAD")) {
                    int src_index = *(const int*)payload->Data;
                    if (src_index != i) {
                        std::swap(_state.frames[src_index], _state.frames[i]);
                    }
                }
                ImGui::EndDragDropTarget();
            }
            ImGui::PopID();
        }

        ImGui::EndChild();

        ImGui::TableSetColumnIndex(1);
        ImGui::BeginChild("FramePreview", ImVec2(0, 0), true);

        if (_state.animation_state.current_frame >= 0 &&
            _state.animation_state.current_frame < _state.frames.size()) {
            SDL_Rect& frame = _state.frames[_state.animation_state.current_frame];
            auto texture = _state.texture_sprite.texture();

            if (texture) {
                float aspect_ratio = static_cast<float>(frame.w) / frame.h;
                float preview_size = ImGui::GetContentRegionAvail().x;
                ImVec2 display_size;

                if (aspect_ratio >= 1.0f) {
                    display_size.x = preview_size;
                    display_size.y = preview_size / aspect_ratio;
                } else {
                    display_size.x = preview_size * aspect_ratio;
                    display_size.y = preview_size;
                }

                ImTextureID tex_id = (ImTextureID)(intptr_t)texture->get();

                ImVec2 uv0(static_cast<float>(frame.x) / texture->width(),
                           static_cast<float>(frame.y) / texture->height());
                ImVec2 uv1(static_cast<float>(frame.x + frame.w) / texture->width(),
                           static_cast<float>(frame.y + frame.h) / texture->height());

                float vertical_padding = (ImGui::GetContentRegionAvail().y - display_size.y) * 0.5f;
                if (vertical_padding > 0.0f) {
                    ImGui::Dummy(ImVec2(0.0f, vertical_padding));
                }

                ImGui::Image(tex_id, display_size, uv0, uv1);
            }
        } else {
            ImGui::Text("No frame selected.");
        }

        ImGui::EndChild();

        ImGui::EndTable();
    }
}

void AnimationPreview::delete_frame(size_t frame_index) {
    if (_state.selected_frames.count(frame_index)) {
        _state.selected_frames.erase(frame_index);
    }
    if (frame_index < _state.frames.size()) {
        _state.frames.erase(_state.frames.begin() + frame_index);
    }
}
}  // namespace components
}  // namespace piksy
