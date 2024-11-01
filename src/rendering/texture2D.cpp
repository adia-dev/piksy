#include <SDL_image.h>

#include <filesystem>
#include <rendering/texture2D.hpp>
#include <stdexcept>

namespace fs = std::filesystem;

namespace piksy {

Texture2D::Texture2D(SDL_Renderer *renderer, const std::string &texture_path) {
    if (renderer == nullptr) {
        throw std::runtime_error("Cannot load texture if the renderer is null.");
    }

    if (!fs::exists(texture_path)) {
        throw std::runtime_error("Failed to load the texture, file does not exists");
    }

    texture.reset(IMG_LoadTexture(renderer, texture_path.c_str()), SDL_DestroyTexture);

    if (texture == nullptr) {
        throw std::runtime_error(
            "SDL_Error: Failed to create a texture from "
            "surface: SDL_CreateTextureFromSurface()");
    }

    if (SDL_QueryTexture(texture.get(), nullptr, nullptr, &width, &height) != 0) {
        throw std::runtime_error(std::string("SDL_QueryTexture Error: ") + SDL_GetError());
    }
}
}  // namespace piksy
