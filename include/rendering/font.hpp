#pragma once

#include <SDL_ttf.h>

#include <memory>

namespace piksy {
namespace rendering {
class Font {
   public:
    explicit Font(const std::string& font_path, int font_size = 24);

    TTF_Font* get() const;

    void cleanup();

   private:
    std::unique_ptr<TTF_Font, decltype(&TTF_CloseFont)> m_font{nullptr, TTF_CloseFont};
};
}  // namespace rendering
}  // namespace piksy
