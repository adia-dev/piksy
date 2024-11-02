#pragma once

#include <filesystem>

#include "core/state.hpp"
#include "rendering/renderer.hpp"

namespace piksy {
namespace components {
class Project {
   public:
    void render(rendering::Renderer& renderer, core::State& state);

   private:
    bool try_select_texture(rendering::Renderer& renderer, const std::filesystem::path& file_path,
                            core::State& state);
};
}  // namespace components
}  // namespace piksy
