#pragma once

#include <rendering/frame.hpp>
#include <string>
#include <vector>

// TODO: Move this in the `animation` namespace
namespace piksy {
namespace rendering {
struct Animation {
    Animation() {}
    std::string name;
    std::vector<Frame> frames;
};
}  // namespace rendering
}  // namespace piksy
