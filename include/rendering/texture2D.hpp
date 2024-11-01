#pragma once

#include <SDL_render.h>

#include <memory>
#include <string>

namespace piksy {
struct Texture2D {
   public:
    Texture2D(SDL_Renderer *renderer, const std::string &texture_path);

    SDL_Texture *get() const { return texture.get(); }

    // TODO: Write custom deleter with debug logs on delete
    std::shared_ptr<SDL_Texture> texture{nullptr, SDL_DestroyTexture};
    int width, height;
};
}  // namespace piksy
