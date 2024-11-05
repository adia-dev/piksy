#include <SDL_error.h>
#include <SDL_image.h>
#include <SDL_render.h>

#include <core/logger.hpp>
#include <filesystem>
#include <rendering/texture2D.hpp>

namespace fs = std::filesystem;

namespace piksy {
namespace rendering {

Texture2D::Texture2D(SDL_Texture* texture) {
    if (texture != nullptr) {
        SDL_QueryTexture(texture, nullptr, nullptr, &_width, &_height);
    }
    _texture.reset(texture);
}

Texture2D::Texture2D(SDL_Renderer* renderer, const std::string& texture_path)
    : _path(texture_path) {
    load(renderer);
}

void Texture2D::reload(SDL_Renderer* renderer) { load(renderer); }

void Texture2D::load(SDL_Renderer* renderer) {
    if (renderer == nullptr) {
        core::Logger::error("Failed to load the texture, the renderer is null");
    }

    if (!fs::exists(_path)) {
        core::Logger::error("Failed to load the texture, file does not exist");
    }

    SDL_Surface* surface = IMG_Load(_path.c_str());
    if (surface == nullptr) {
        core::Logger::error("Failed to load the image %s into a surface: %s", _path.c_str(),
                            IMG_GetError());
    }

    SDL_Surface* converted_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA8888, 0);
    SDL_FreeSurface(surface);
    surface = converted_surface;
    if (surface == nullptr) {
        core::Logger::error("Failed to convert surface to RGBA8888 format: %s", SDL_GetError());
        throw "unreachable";
    }

    _pitch = surface->pitch;
    _width = surface->w;
    _height = surface->h;

    _texture.reset(SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                     SDL_TEXTUREACCESS_STREAMING, _width, _height));
    SDL_SetTextureBlendMode(_texture.get(), SDL_BLENDMODE_BLEND);
    if (_texture == nullptr) {
        core::Logger::error("Failed to create a texture from a surface: %s", SDL_GetError());
    }

    void* pixels;
    int pitch;
    if (SDL_LockTexture(_texture.get(), nullptr, &pixels, &pitch) != 0) {
        core::Logger::error("Failed to lock the texture: %s", SDL_GetError());
    }

    Uint8* dst = static_cast<Uint8*>(pixels);
    Uint8* src = static_cast<Uint8*>(surface->pixels);
    for (int row = 0; row < _height; ++row) {
        memcpy(dst + row * pitch, src + row * surface->pitch, surface->pitch);
    }

    SDL_UnlockTexture(_texture.get());
    SDL_FreeSurface(surface);
}

SDL_Texture* Texture2D::get() const { return _texture.get(); }

int Texture2D::width() const { return _width; }

int Texture2D::height() const { return _height; }

const std::string& Texture2D::path() const { return _path; }

void Texture2D::set_path(const std::string& path) { _path = path; }
}  // namespace rendering
}  // namespace piksy
