#pragma once

#include <core/state.hpp>

namespace piksy {
namespace components {
class Inspector {
   public:
    void render(core::State& state);

   private:
    void render_sprite_properties(core::State& state);
    void help_marker(const std::string& desc);
};
}  // namespace components
}  // namespace piksy
