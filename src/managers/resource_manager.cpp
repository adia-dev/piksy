#include <filesystem>
#include <iostream>
#include <managers/resource_manager.hpp>
#include <stdexcept>

namespace fs = std::filesystem;

namespace piksy {
namespace managers {

ResourceManager &ResourceManager::get() {
    static ResourceManager instance;
    return instance;
}

void ResourceManager::cleanup() {
    ResourceManager &resource_manager = get();
    resource_manager._textures.clear();
    resource_manager._fonts.clear();

    std::cout << "Resource manager cleaned up\n";
}

void ResourceManager::load_texture(SDL_Renderer *renderer, const std::string &texture_path) {
    get_texture(renderer, texture_path);
}

std::shared_ptr<rendering::Texture2D> ResourceManager::get_texture(
    SDL_Renderer *renderer, const std::string &texture_path) {
    ResourceManager &resource_manager = get();

    auto texture_found = resource_manager._textures.find(texture_path);
    if (texture_found != resource_manager._textures.end()) {
        return texture_found->second;
    }

    if (!fs::exists(texture_path)) {
        throw std::runtime_error("Failed to get the texture: File not found, Path: " +
                                 texture_path);
    }

    resource_manager._textures.emplace(
        texture_path, std::make_shared<rendering::Texture2D>(renderer, texture_path));
    return resource_manager._textures.at(texture_path);
}

void ResourceManager::load_font(SDL_Renderer *renderer, const std::string &font_path) {
    get_font(renderer, font_path);
}

std::shared_ptr<rendering::Font> ResourceManager::get_font(SDL_Renderer *renderer,
                                                           const std::string &font_path) {
    ResourceManager &resource_manager = get();

    auto font_found = resource_manager._fonts.find(font_path);
    if (font_found != resource_manager._fonts.end()) {
        return font_found->second;
    }

    if (!fs::exists(font_path)) {
        throw std::runtime_error("Failed to get the font: File not found, Path: " + font_path);
    }

    resource_manager._fonts.emplace(font_path, std::make_shared<rendering::Font>(font_path));
    return resource_manager._fonts.at(font_path);
}

}  // namespace managers
}  // namespace piksy
