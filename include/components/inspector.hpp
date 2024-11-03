#pragma once

#include <components/ui_component.hpp>
#include <core/state.hpp>

namespace piksy {
namespace components {
class Inspector : public UIComponent {
   public:
    void update() override;
    void render(core::State& state) override;

   private:
    void render_sprite_properties(core::State& state);
    void help_marker(const std::string& desc);
};
}  // namespace components
}  // namespace piksy
