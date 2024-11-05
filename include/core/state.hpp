#pragma once

#include <filesystem>
#include <rendering/sprite.hpp>

namespace piksy {
namespace core {
struct State {
    rendering::Sprite texture_sprite;
    float replacement_color[4]{0.0f, 0.0f, 0.0f, 0.0f};
    std::filesystem::path current_path = RESOURCE_DIR;
};
}  // namespace core
}  // namespace piksy
