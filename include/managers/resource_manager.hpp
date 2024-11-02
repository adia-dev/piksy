#pragma once

#include <SDL_render.h>

#include <rendering/font.hpp>
#include <rendering/texture2D.hpp>
#include <unordered_map>

namespace piksy {
namespace managers {
class ResourceManager {
   public:
    static ResourceManager &get();

    static void load_texture(SDL_Renderer *renderer, const std::string &texture_path);
    static std::shared_ptr<rendering::Texture2D> get_texture(SDL_Renderer *renderer,
                                                             const std::string &texture_path);

    static void load_font(SDL_Renderer *renderer, const std::string &font_path);
    static std::shared_ptr<rendering::Font> get_font(SDL_Renderer *renderer,
                                                     const std::string &font_path);

    static void cleanup();

   private:
    ResourceManager() = default;
    std::unordered_map<std::string, std::shared_ptr<rendering::Texture2D>> _textures;
    std::unordered_map<std::string, std::shared_ptr<rendering::Font>> _fonts;
};
}  // namespace managers
}  // namespace piksy
