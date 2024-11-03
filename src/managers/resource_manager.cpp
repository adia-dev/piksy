#include <core/logger.hpp>
#include <filesystem>
#include <managers/resource_manager.hpp>

namespace fs = std::filesystem;

namespace piksy {
namespace managers {

void ResourceManager::cleanup() {
    _textures.clear();
    _fonts.clear();

    core::Logger::debug("Resource manager cleanup up successfully");
}

void ResourceManager::load_texture(const std::string &texture_path) { get_texture(texture_path); }

std::shared_ptr<rendering::Texture2D> ResourceManager::get_texture(
    const std::string &texture_path) {
    auto texture_found = _textures.find(texture_path);
    if (texture_found != _textures.end()) {
        return texture_found->second;
    }

    if (!fs::exists(texture_path)) {
        core::Logger::fatal("Failed to get the texture: File not found, Path: %s",
                            texture_path.c_str());
    }

    _textures.emplace(texture_path,
                      std::make_shared<rendering::Texture2D>(_renderer.get(), texture_path));
    return _textures.at(texture_path);
}

void ResourceManager::load_font(const std::string &font_path) { get_font(font_path); }

std::shared_ptr<rendering::Font> ResourceManager::get_font(const std::string &font_path) {
    auto font_found = _fonts.find(font_path);
    if (font_found != _fonts.end()) {
        return font_found->second;
    }

    if (!fs::exists(font_path)) {
        core::Logger::fatal("Failed to get the font: File not found, Path: %s", font_path.c_str());
    }

    _fonts.emplace(font_path, std::make_shared<rendering::Font>(font_path));
    return _fonts.at(font_path);
}

}  // namespace managers
}  // namespace piksy
