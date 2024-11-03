#include <SDL_ttf.h>

#include <core/logger.hpp>
#include <rendering/font.hpp>

namespace piksy {
namespace rendering {

Font::Font(const std::string& font_path, int font_size) {
    _font.reset(TTF_OpenFont(font_path.c_str(), font_size));

    if (_font == nullptr) {
        core::Logger::fatal("Failed to load font: %s", TTF_GetError());
    }
}

TTF_Font* Font::get() const { return _font.get(); }

}  // namespace rendering
}  // namespace piksy
