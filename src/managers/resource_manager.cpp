#include <filesystem>
#include <managers/resource_manager.hpp>
#include <stdexcept>

namespace fs = std::filesystem;

namespace piksy {

ResourceManager &ResourceManager::get() {
    static ResourceManager instance;
    return instance;
}

void ResourceManager::load_texture(SDL_Renderer *renderer, const std::string &texture_path) {
    get_texture(renderer, texture_path);
}

std::shared_ptr<Texture2D> ResourceManager::get_texture(SDL_Renderer *renderer,
                                                        const std::string &texture_path) {
    ResourceManager &resource_manager = get();

    auto texture_found = resource_manager._textures.find(texture_path);
    if (texture_found != resource_manager._textures.end()) {
        return texture_found->second;
    }

    if (!fs::exists(texture_path)) {
        throw std::runtime_error("Failed to get the texture: File not found, Path: " +
                                 texture_path);
    }

    resource_manager._textures.emplace(texture_path,
                                       std::make_shared<Texture2D>(renderer, texture_path));
    return resource_manager._textures.at(texture_path);
}

bool ResourceManager::unload_texture(const std::string &texture_path) {
    ResourceManager &resource_manager = get();

    auto texture_found = resource_manager._textures.find(texture_path);
    if (texture_found != resource_manager._textures.end()) {
        return false;
    }

    resource_manager._textures.erase(texture_path);
    return true;
}

}  // namespace piksy
