#pragma once

#include <imgui.h>

#include <core/config.hpp>

#include "rendering/renderer.hpp"
#include "rendering/window.hpp"

namespace piksy {
namespace contexts {
class ImGuiContext {
   public:
    ~ImGuiContext();

    void init(const core::ImGuiConfig& config, rendering::Window& window,
              rendering::Renderer& renderer);
    void cleanup();

   private:
    bool m_cleaned_up = true;
};

}  // namespace contexts
}  // namespace piksy
