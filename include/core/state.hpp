#pragma once

#include <filesystem>
#include <rendering/sprite.hpp>

#include "core/logger.hpp"

namespace piksy {
namespace core {
struct MouseState {
    ImVec2 start_pos;
    ImVec2 current_pos;
    bool is_pressed = false;
    bool is_panning = false;
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

struct State {
    rendering::Sprite texture_sprite;
    float replacement_color[4]{0.0f, 0.0f, 0.0f, 0.0f};
    std::filesystem::path current_path = RESOURCE_DIR;
    int viewport_grid_cell_size = 20;

    MouseState mouse_state;
    ZoomState zoom_state;
    PanState pan_state;

    // Frames rects from the viewport
    std::vector<SDL_Rect> frames;

    ~State() { Logger::debug("State cleanup up"); }
};
}  // namespace core
}  // namespace piksy
