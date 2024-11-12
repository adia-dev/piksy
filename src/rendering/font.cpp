#include <SDL_ttf.h>

#include <core/logger.hpp>
#include <rendering/font.hpp>

namespace piksy {
namespace rendering {

Font::Font(const std::string& font_path, int font_size) {
    m_font.reset(TTF_OpenFont(font_path.c_str(), font_size));

    if (m_font == nullptr) {
        core::Logger::error("Failed to load font: %s", TTF_GetError());
    }
}

TTF_Font* Font::get() const { return m_font.get(); }

}  // namespace rendering
}  // namespace piksy
