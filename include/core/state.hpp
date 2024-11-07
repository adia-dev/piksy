#pragma once

#include <filesystem>
#include <rendering/sprite.hpp>

#include "core/logger.hpp"

namespace piksy {
namespace core {
struct State {
    rendering::Sprite texture_sprite;
    float replacement_color[4]{0.0f, 0.0f, 0.0f, 0.0f};
    std::filesystem::path current_path = RESOURCE_DIR;
    int viewport_grid_cell_size = 20;
    std::vector<SDL_Rect> frames;

    ~State() { Logger::debug("State cleanup up"); }
};
}  // namespace core
}  // namespace piksy
