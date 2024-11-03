#pragma once

#include <components/ui_component.hpp>
#include <core/state.hpp>
#include <filesystem>
#include <rendering/renderer.hpp>

#include "managers/resource_manager.hpp"

namespace piksy {
namespace components {

class Project : public UIComponent {
   public:
    explicit Project(rendering::Renderer& renderer, managers::ResourceManager& resource_manager);

    void update() override;
    void render(core::State& state) override;

   private:
    bool try_select_texture(const std::filesystem::path& file_path, core::State& state);
    void render_file_browser(core::State& state);

    rendering::Renderer& _renderer;
    managers::ResourceManager& _resource_manager;
};

}  // namespace components
}  // namespace piksy
