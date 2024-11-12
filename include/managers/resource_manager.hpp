#pragma once

#include <SDL_render.h>

#include <rendering/font.hpp>
#include <rendering/texture2D.hpp>
#include <unordered_map>

#include "rendering/renderer.hpp"

namespace piksy {
namespace managers {
class ResourceManager {
   public:
    explicit ResourceManager(rendering::Renderer &renderer) : m_renderer(renderer) {}

    void load_texture(const std::string &texture_path);
    std::shared_ptr<rendering::Texture2D> get_texture(const std::string &texture_path);

    void load_font(const std::string &font_path);
    std::shared_ptr<rendering::Font> get_font(const std::string &font_path);

    void cleanup();

   private:
    rendering::Renderer &m_renderer;
    std::unordered_map<std::string, std::shared_ptr<rendering::Texture2D>> m_textures;
    std::unordered_map<std::string, std::shared_ptr<rendering::Font>> m_fonts;
};
}  // namespace managers
}  // namespace piksy
