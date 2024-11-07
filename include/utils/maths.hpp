#pragma once

#include "imgui.h"

namespace piksy {
namespace utils {
namespace maths {

inline static ImVec2 lerp(const ImVec2& lhs, const ImVec2& rhs, float t) {
    return {lhs.x + t * (rhs.x - lhs.x), lhs.y + t * (rhs.y - lhs.y)};
}

inline static float lerp(float lhs, float rhs, float t) { return lhs + t * (rhs - lhs); }

}  // namespace maths
}  // namespace utils
}  // namespace piksy
