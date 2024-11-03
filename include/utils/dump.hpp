#pragma once

#include <imgui.h>

#include <cstdio>
#include <string>

namespace piksy {
namespace utils {
inline void dump(const ImVec2& vec, const std::string& label = "") {
    printf("%s(%.2f, %.2f)\n", (label != "" ? label + ": " : "").c_str(), vec.x, vec.y);
}
inline void dump(const ImVec4& vec, const std::string& label = "") {
    printf("%s(%.2f, %.2f, %.2f, %.2f)\n", (label != "" ? label + ": " : "").c_str(), vec.x, vec.y,
           vec.w, vec.z);
}
}  // namespace utils
}  // namespace piksy
