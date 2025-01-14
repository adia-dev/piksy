#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <command/export_texture_command.hpp>
#include <core/logger.hpp>

namespace piksy {
namespace commands {

namespace fs = std::filesystem;

ExportTextureCommand::ExportTextureCommand(core::State& state, fs::path output_path)
    : m_state(state), m_output_path(std::move(output_path)) {}

void ExportTextureCommand::execute() {
    core::Logger::info("Exporting texture to file: %s", m_output_path.c_str());

    // 1) Check if we have a valid texture
    auto sprite_texture = m_state.texture_sprite.texture();
    if (!sprite_texture) {
        core::Logger::error("No texture loaded, cannot export.");
        return;
    }

    SDL_Texture* texture = sprite_texture->get();
    if (!texture) {
        core::Logger::error("Sprite has no SDL_Texture, cannot export.");
        return;
    }

    // 2) Query the texture for width/height
    int w = 0, h = 0;
    if (SDL_QueryTexture(texture, nullptr, nullptr, &w, &h) != 0) {
        core::Logger::error("Failed to query texture: %s", SDL_GetError());
        return;
    }

    // 3) Create a surface to hold the pixel data
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGBA8888);
    if (!surface) {
        core::Logger::error("Failed to create surface: %s", SDL_GetError());
        return;
    }

    // 4) Lock the texture and copy pixels into the surface
    void* pixels = nullptr;
    int pitch = 0;
    if (SDL_LockTexture(texture, nullptr, &pixels, &pitch) != 0) {
        core::Logger::error("Failed to lock texture for exporting: %s", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }

    // Copy line by line: each row is 'pitch' bytes in the texture
    // The surface->pitch may differ from 'pitch' but they are usually both w*4 in RGBA8888.
    // We'll do a row copy for safety.
    Uint8* dstPixels = static_cast<Uint8*>(surface->pixels);
    Uint8* srcPixels = static_cast<Uint8*>(pixels);
    int copy_bytes_per_row = (pitch < surface->pitch) ? pitch : surface->pitch;
    for (int row = 0; row < h; ++row) {
        std::memcpy(dstPixels + row * surface->pitch, srcPixels + row * pitch, copy_bytes_per_row);
    }

    SDL_UnlockTexture(texture);

    // 5) Create parent directories if needed
    if (!fs::exists(m_output_path.parent_path()) && !m_output_path.parent_path().empty()) {
        try {
            fs::create_directories(m_output_path.parent_path());
        } catch (const fs::filesystem_error& e) {
            core::Logger::error("Failed to create directories for export: %s", e.what());
            SDL_FreeSurface(surface);
            return;
        }
    }

    // 6) Use SDL_image to save as PNG
    // This requires SDL_image 2.0.2 or higher for IMG_SavePNG.
    if (IMG_SavePNG(surface, m_output_path.string().c_str()) != 0) {
        core::Logger::error("IMG_SavePNG failed: %s", IMG_GetError());
    } else {
        core::Logger::info("Texture exported successfully to: %s", m_output_path.c_str());
    }

    SDL_FreeSurface(surface);
}

}  // namespace commands
}  // namespace piksy
