#include <SDL_image.h>

#include <filesystem>
#include <rendering/texture2D.hpp>
#include <stdexcept>

namespace fs = std::filesystem;

namespace piksy {
namespace rendering {

Texture2D::Texture2D(SDL_Renderer *renderer, const std::string &texture_path)
    : _texture_path(texture_path) {
    load(renderer);
}

void Texture2D::reload(SDL_Renderer *renderer) { load(renderer); }

void Texture2D::load(SDL_Renderer *renderer) {
    if (renderer == nullptr) {
        throw std::runtime_error("Cannot load texture if the renderer is null.");
    }

    if (!fs::exists(_texture_path)) {
        throw std::runtime_error("Failed to load the texture, file does not exists");
    }

    _texture.reset(IMG_LoadTexture(renderer, _texture_path.c_str()));

    if (_texture == nullptr) {
        throw std::runtime_error(
            "SDL_Error: Failed to create a texture from "
            "surface: SDL_CreateTextureFromSurface()");
    }

    if (SDL_QueryTexture(_texture.get(), nullptr, nullptr, &_width, &_height) != 0) {
        throw std::runtime_error(std::string("SDL_QueryTexture Error: ") + SDL_GetError());
    }
}

SDL_Texture *Texture2D::get() const { return _texture.get(); }

int Texture2D::width() const { return _width; }

int Texture2D::height() const { return _height; }
}  // namespace rendering
}  // namespace piksy
