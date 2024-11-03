#pragma once

#include <SDL_ttf.h>

#include <memory>

namespace piksy {
namespace rendering {
class Font {
   public:
    Font(const std::string& font_path, int font_size = 24);

    TTF_Font* get() const;

   private:
    std::unique_ptr<TTF_Font, decltype(&TTF_CloseFont)> _font{nullptr, TTF_CloseFont};
};
}  // namespace rendering
}  // namespace piksy
