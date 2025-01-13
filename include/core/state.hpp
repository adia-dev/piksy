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
#include "rendering/frame.hpp"

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
    COLOR_SWAP,    // Color swapping tool, swap a color of a texture for another
    BRUSH,         // Brush tool for painting or drawing
    EXTRACT,       // Custom extraction tool
    VIEW,          // View tool for changing camera angles or perspective
    LASSO_SELECT,  // Lasso selection tool for freeform selections
    ZOOM_IN,       // Zoom in tool for changing viewport magnification
    ZOOM_OUT,      // Zoom out tool for changing viewport magnification
    COUNT          // Count of all the tools available
};

// TODO: Move this the f*ck outta here
inline const char* tool_to_string(Tool tool) {
    switch (tool) {
        case Tool::SELECT:
            return ICON_FA_HAND_POINTER_O;
        case Tool::MOVE:
            return ICON_FA_ARROWS;
        case Tool::SCALE:
            return ICON_FA_ARROWS_V;
        case Tool::ERASE:
            return ICON_FA_ERASER;
        case Tool::MEASURE:
            return ICON_FA_ANGLE_RIGHT;
        case Tool::SNAP:
            return ICON_FA_MAGNET;
        case Tool::PAN:
            return ICON_FA_PAPER_PLANE_O;
        case Tool::COLOR_SWAP:
            return ICON_FA_TINT;
        case Tool::BRUSH:
            return ICON_FA_PAINT_BRUSH;
        case Tool::EXTRACT:
            return ICON_FA_SQUARE_O;
        case Tool::VIEW:
            return ICON_FA_EYE;
        case Tool::LASSO_SELECT:
            return ICON_FA_PENCIL_SQUARE_O;
        case Tool::ZOOM_IN:
            return ICON_FA_SEARCH_PLUS;
        case Tool::ZOOM_OUT:
            return ICON_FA_SEARCH_MINUS;
        case Tool::COUNT:
        default:
            return "Unknown";
    }
}

inline const char* tool_to_icon(Tool tool) {
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
        case Tool::ZOOM_IN:
            return "ZOOM_IN";
        case Tool::ZOOM_OUT:
            return "ZOOM_OUT";
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

    Tool current_tool = Tool::PAN;
};
}  // namespace core
}  // namespace piksy
