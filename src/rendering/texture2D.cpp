#include <SDL_error.h>
#include <SDL_image.h>
#include <SDL_render.h>

#include <core/logger.hpp>
#include <filesystem>
#include <rendering/texture2D.hpp>
#include <stdexcept>

namespace fs = std::filesystem;

namespace piksy {
namespace rendering {

Texture2D::Texture2D(SDL_Texture* texture) {
    if (texture != nullptr) {
        SDL_QueryTexture(texture, nullptr, nullptr, &m_width, &m_height);
    }
    m_texture.reset(texture);
}

Texture2D::Texture2D(SDL_Renderer* renderer, const std::string& texture_path)
    : m_path(texture_path) {
    load(renderer);
}

void Texture2D::reload(SDL_Renderer* renderer) { load(renderer); }

void Texture2D::load(SDL_Renderer* renderer) {
    if (renderer == nullptr) {
        core::Logger::error("Failed to load the texture, the renderer is null");
        throw std::runtime_error("Failed to load the texture, the renderer is null");
    }

    if (!fs::exists(m_path)) {
        core::Logger::error("Failed to load the texture, file does not exist");
        throw std::runtime_error("Failed to load the texture, file does not exist");
    }

    SDL_Surface* surface = IMG_Load(m_path.c_str());
    if (surface == nullptr) {
        core::Logger::error("Failed to load the image %s into a surface: %s", m_path.c_str(),
                            IMG_GetError());
        throw std::runtime_error(std::string("Failed to load the image into a surface: ") +
                                 IMG_GetError());
    }

    SDL_Surface* converted_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA8888, 0);
    SDL_FreeSurface(surface);
    surface = converted_surface;
    if (surface == nullptr) {
        core::Logger::error("Failed to convert surface to RGBA8888 format: %s", SDL_GetError());
        throw "unreachable";
    }

    m_pitch = surface->pitch;
    m_width = surface->w;
    m_height = surface->h;

    m_texture.reset(SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                     SDL_TEXTUREACCESS_STREAMING, m_width, m_height));
    SDL_SetTextureBlendMode(m_texture.get(), SDL_BLENDMODE_BLEND);
    if (m_texture == nullptr) {
        core::Logger::error("Failed to create a texture from a surface: %s", SDL_GetError());
        throw std::runtime_error(std::string("Failed to create a texture from a surface: %s") +
                                 SDL_GetError());
    }

    void* pixels;
    int pitch;
    if (SDL_LockTexture(m_texture.get(), nullptr, &pixels, &pitch) != 0) {
        core::Logger::error("Failed to lock the texture: %s", SDL_GetError());
        throw std::runtime_error(std::string("Failed to lock the texture: %s") + SDL_GetError());
    }

    Uint8* dst = static_cast<Uint8*>(pixels);
    Uint8* src = static_cast<Uint8*>(surface->pixels);
    for (int row = 0; row < m_height; ++row) {
        memcpy(dst + row * pitch, src + row * surface->pitch, surface->pitch);
    }

    SDL_UnlockTexture(m_texture.get());
    SDL_FreeSurface(surface);
}

SDL_Texture* Texture2D::get() const { return m_texture.get(); }

int Texture2D::width() const { return m_width; }

int Texture2D::height() const { return m_height; }

const std::string& Texture2D::path() const { return m_path; }

void Texture2D::set_path(const std::string& path) { m_path = path; }
}  // namespace rendering
}  // namespace piksy
