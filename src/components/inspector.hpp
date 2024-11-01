#pragma once

#include "imgui.h"
namespace piksy {
class Inspector {
public:
  static void render() {
    ImGui::Begin("Inspector");
    ImGui::Text("EHEHEHHEE");
    ImGui::End();
  }

private:
};
} // namespace piksy
