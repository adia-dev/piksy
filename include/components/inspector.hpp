#pragma once

#include <components/ui_component.hpp>
#include <core/state.hpp>
#include <rendering/renderer.hpp>

namespace piksy {
namespace components {
class Inspector : public UIComponent {
   public:
    explicit Inspector(core::State& state, rendering::Renderer& renderer);
    void update() override;
    void render() override;

   private:
    void render_sprite_properties();
    void render_texture_properties(std::shared_ptr<rendering::Texture2D> texture);
    void help_marker(const std::string& desc);

    rendering::Renderer& _renderer;
};
}  // namespace components
}  // namespace piksy
