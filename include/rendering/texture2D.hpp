#pragma once

#include <SDL_render.h>

#include <memory>
#include <string>

namespace piksy {
namespace rendering {
class Texture2D {
   public:
    explicit Texture2D(SDL_Texture *texture);
    Texture2D(SDL_Renderer *renderer, const std::string &texture_path);

    SDL_Texture *get() const;
    int width() const;
    int height() const;

    void reload(SDL_Renderer *renderer);

   private:
    // TODO: Write custom deleter with debug logs on delete
    std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> _texture{nullptr,
                                                                         SDL_DestroyTexture};

    int _width, _height, _pitch;

    // NOTE: Do I actually need this ?
    std::string _texture_path;

    void load(SDL_Renderer *renderer);
};
}  // namespace rendering
}  // namespace piksy
