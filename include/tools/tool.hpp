#pragma once

#include "icons/IconsFontAwesome4.h"
#include "icons/IconsMaterialDesign.h"
namespace piksy {
namespace tools {

enum class Tool {
    SELECT,        // Basic selection tool for choosing elements
    MOVE,          // Move tool for repositioning objects
    SCALE,         // Scale tool for resizing objects
    ERASE,         // Erase tool for removing parts of objects
    MEASURE,       // Measure tool for checking distances or dimensions
    PAN,           // Pan tool for moving the viewport
    COLOR_SWAP,    // Color swapping tool, swap a color of a texture for another
    BRUSH,         // Brush tool for painting or drawing
    AUTO_EXTRACT,  // Automatically extract frames
    EXTRACT,       // Extract frames from a selection rect
    VIEW,          // View tool for changing camera angles or perspective
    LASSO_SELECT,  // Lasso selection tool for freeform selections
    ZOOM_IN,       // Zoom in tool for changing viewport magnification
    ZOOM_OUT,      // Zoom out tool for changing viewport magnification
    COUNT          // Count of all the tools available
};

// TODO: Move this the f*ck outta here
inline const char* tool_to_icon(Tool tool) {
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
        case Tool::AUTO_EXTRACT:
            return ICON_MD_AUTO_FIX_HIGH;
        case Tool::PAN:
            return ICON_MD_PAN_TOOL;
        case Tool::COLOR_SWAP:
            return ICON_MD_INVERT_COLORS_OFF;
        case Tool::BRUSH:
            return ICON_MD_BRUSH;
        case Tool::EXTRACT:
            return ICON_MD_HIGHLIGHT_ALT;
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
        case Tool::AUTO_EXTRACT:
            return "AUTO EXTRACT";
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

}  // namespace tools
}  // namespace piksy
