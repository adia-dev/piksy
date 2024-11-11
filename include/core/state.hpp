#pragma once

#include <SDL_rect.h>

#include <core/logger.hpp>
#include <filesystem>
#include <functional>
#include <rendering/sprite.hpp>
#include <unordered_set>

namespace piksy {
namespace core {

enum class Tool {
    SELECT,        // Basic selection tool for choosing elements
    MOVE,          // Move tool for repositioning objects
    SCALE,         // Scale tool for resizing objects
    ERASE,         // Erase tool for removing parts of objects
    MEASURE,       // Measure tool for checking distances or dimensions
    SNAP,          // Snap tool for aligning objects precisely
    PAN,           // Pan tool for moving the viewport
    ZOOM,          // Zoom tool for changing viewport magnification
    COLOR_SWAP,    // Color swapping tool, swap a color of a texture for another
    BRUSH,         // Brush tool for painting or drawing
    EXTRACT,       // Custom extraction tool
    VIEW,          // View tool for changing camera angles or perspective
    LASSO_SELECT,  // Lasso selection tool for freeform selections
    COUNT          // Count of all the tools available
};

// TODO: Move this the f*ck outta here
inline const char* tool_to_string(Tool tool) {
    switch (tool) {
        case Tool::SELECT:
            return "SELECT";
        case Tool::MOVE:
            return "MOVE";
        case Tool::SCALE:
            return "SCALE";
        case Tool::ERASE:
            return "ERASE";
        case Tool::MEASURE:
            return "MEASURE";
        case Tool::SNAP:
            return "SNAP";
        case Tool::PAN:
            return "PAN";
        case Tool::ZOOM:
            return "ZOOM";
        case Tool::COLOR_SWAP:
            return "COLOR SWAP";
        case Tool::BRUSH:
            return "BRUSH";
        case Tool::EXTRACT:
            return "EXTRACT";
        case Tool::VIEW:
            return "VIEW";
        case Tool::LASSO_SELECT:
            return "LASSO_SELECT";
        case Tool::COUNT:
            return "COUNT";
        default:
            return "Unknown";
    }
}

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

struct AnimationState {
    bool is_playing = false;
    float fps = 24.0f;
    int current_frame = 0;
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

    // Frames rects from the viewport
    std::vector<SDL_Rect> frames;
    std::unordered_set<size_t> selected_frames;

    Tool current_tool = Tool::PAN;

    ~State() { Logger::debug("State cleanup up"); }
};
}  // namespace core
}  // namespace piksy
