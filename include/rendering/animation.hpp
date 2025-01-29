#pragma once

#include <rendering/frame.hpp>
#include <string>
#include <string_view>
#include <vector>

// TODO: Move this in the `animation` namespace
namespace piksy {
namespace rendering {
struct Animation {
    Animation(const std::string& name) : name(name) {}
    std::string_view name;
    std::vector<Frame> frames;
};
}  // namespace rendering
}  // namespace piksy
