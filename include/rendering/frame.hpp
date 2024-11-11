#pragma once

namespace piksy {
namespace rendering {
struct Frame {
    Frame(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}

    int x, y, w, h;
    bool flipped = false;
};
}  // namespace rendering
}  // namespace piksy
