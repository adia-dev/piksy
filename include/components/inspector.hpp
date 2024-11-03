#pragma once

#include <core/state.hpp>

#include "imgui.h"

namespace piksy {
namespace components {
class Inspector {
   public:
    void render(core::State& state);

   private:
    void render_sprite(core::State& state);
    void help_marker(const std::string& desc) {
        ImGui::TextDisabled("(?)");
        if (ImGui::BeginItemTooltip()) {
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc.c_str());
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }
};
}  // namespace components
}  // namespace piksy
