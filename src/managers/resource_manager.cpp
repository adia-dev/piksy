#include <core/logger.hpp>
#include <filesystem>
#include <managers/resource_manager.hpp>
#include <stdexcept>

namespace fs = std::filesystem;

namespace piksy {
namespace managers {

void ResourceManager::cleanup() {
    for (auto it = m_textures.begin(); it != m_textures.end(); ++it) {
        core::Logger::debug("Cleaning up texture: %s", it->first.c_str());
    }
    m_textures.clear();

    for (auto it = m_fonts.begin(); it != m_fonts.end(); ++it) {
        core::Logger::debug("Cleaning up font: %s", it->first.c_str());
    }
    m_fonts.clear();

    core::Logger::debug("Resource manager cleaned up successfully");
}

void ResourceManager::load_texture(const std::string &texture_path) { get_texture(texture_path); }

std::shared_ptr<rendering::Texture2D> ResourceManager::get_texture(
    const std::string &texture_path) {
    auto texture_found = m_textures.find(texture_path);
    if (texture_found != m_textures.end()) {
        return texture_found->second;
    }

    if (!fs::exists(texture_path)) {
        core::Logger::error("Failed to get the texture: File not found, Path: %s",
                            texture_path.c_str());
        throw std::runtime_error("Failed to get the texture: File not found at: " + texture_path);
    }

    core::Logger::debug("Loading texture: %s", texture_path.c_str());
    m_textures.emplace(texture_path,
                       std::make_shared<rendering::Texture2D>(m_renderer.get(), texture_path));
    return m_textures.at(texture_path);
}

void ResourceManager::load_font(const std::string &font_path) { get_font(font_path); }

std::shared_ptr<rendering::Font> ResourceManager::get_font(const std::string &font_path) {
    auto font_found = m_fonts.find(font_path);
    if (font_found != m_fonts.end()) {
        return font_found->second;
    }

    if (!fs::exists(font_path)) {
        core::Logger::error("Failed to get the font: File not found, Path: %s", font_path.c_str());
        throw std::runtime_error("Failed to get the font: File not found at: " + font_path);
    }

    core::Logger::debug("Loading font: %s", font_path.c_str());
    m_fonts.emplace(font_path, std::make_shared<rendering::Font>(font_path));
    return m_fonts.at(font_path);
}

}  // namespace managers
}  // namespace piksy
