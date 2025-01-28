#pragma once

#include <SDL_rect.h>
#include <icons/IconsFontAwesome4.h>

#include <core/logger.hpp>
#include <filesystem>
#include <rendering/sprite.hpp>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "rendering/animation.hpp"
#include "tools/tool.hpp"

namespace piksy {
namespace core {

struct MouseState {
    ImVec2 start_pos;
    ImVec2 current_pos;
    bool is_pressed = false;
    bool is_panning = false;
    bool is_dragging = false;
};

struct ZoomState {
    float current_scale = 1.0f;
    float target_scale = 1.0f;
    float zoom_speed = 0.1f;
};

struct PanState {
    ImVec2 current_offset;
    ImVec2 target_offset;
    float pan_speed = 0.7f;
};

struct AnimationState {
    bool is_playing = false;
    float fps = 24.0f;
    int current_frame = 0;
    float frame_duration = 1.0f / fps;
    float timer = 0.0f;

    std::unordered_map<std::string, rendering::Animation> animations;
    std::string current_animation;

    std::unordered_set<size_t> selected_frames;

    rendering::Animation& get_animation(const std::string& animation_name) {
        return animations[animation_name];
    }

    rendering::Animation& get_current_animation() { return animations[current_animation]; }
};

struct ViewportState {
    ImVec2 size;
    SDL_Rect selection_rect;
    int grid_cell_size = 20;
};

struct State {
    rendering::Sprite texture_sprite;
    float replacement_color[4]{0.0f, 0.0f, 0.0f, 0.0f};
    std::filesystem::path current_path = RESOURCE_DIR;

    MouseState mouse_state;
    ZoomState zoom_state;
    PanState pan_state;
    AnimationState animation_state;
    ViewportState viewport_state;

    float delta_time;
    float fps;

    tools::Tool current_tool = tools::Tool::PAN;
};
}  // namespace core
}  // namespace piksy
