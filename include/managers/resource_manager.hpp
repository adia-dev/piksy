#pragma once

#include <SDL_render.h>

#include <rendering/texture2D.hpp>
#include <unordered_map>

namespace piksy {
class ResourceManager {
   public:
    static ResourceManager &get();
    static void load_texture(SDL_Renderer *renderer, const std::string &texture_path);

    static std::shared_ptr<Texture2D> get_texture(SDL_Renderer *renderer,
                                                  const std::string &texture_path);

    static bool unload_texture(const std::string &texture_path);

   private:
    ResourceManager() = default;
    std::unordered_map<std::string, std::shared_ptr<Texture2D>> _textures;
};
}  // namespace piksy
