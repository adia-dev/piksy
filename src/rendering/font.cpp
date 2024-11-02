#include <iostream>
#include <rendering/font.hpp>
#include <stdexcept>

#include "SDL_ttf.h"

namespace piksy {
namespace rendering {

Font::Font(const std::string& font_path, int font_size) {
    _font.reset(TTF_OpenFont(font_path.c_str(), font_size));

    if (_font == nullptr) {
        std::cout << "Failed to load font: " << TTF_GetError() << std::endl;
        throw std::runtime_error("Failed to load font. TTF_OpenFont()");
    }
}

TTF_Font* Font::get() const { return _font.get(); }

}  // namespace rendering
}  // namespace piksy
