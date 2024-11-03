#include <imgui.h>

#include <components/console.hpp>

namespace piksy {
namespace components {

void Console::update() {}

void Console::render(core::State& state) {
    ImGui::Begin("Console");
    ImGui::End();
}

}  // namespace components
}  // namespace piksy
