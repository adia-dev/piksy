#include <contexts/sdl_context.hpp>

#include "core/logger.hpp"

namespace piksy {
namespace contexts {

SDLContext::~SDLContext() {
    if (m_cleaned_up) {
        return;
    }

    TTF_Quit();
    SDL_Quit();

    core::Logger::debug("SDL Context successfully cleaned up");
}

void SDLContext::init(const core::WindowConfig& config) {
    if (SDL_Init(config.init_flags) != 0) {
        core::Logger::fatal("Error initializing SDL: %s", SDL_GetError());
    }

    if (TTF_Init() < 0) {
        core::Logger::fatal("Error initializing SDL_ttf: %s", TTF_GetError());
    }

#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif
}

void SDLContext::cleanup(rendering::Window& window, rendering::Renderer& renderer) {
    TTF_Quit();

    SDL_DestroyRenderer(renderer.get());
    SDL_DestroyWindow(window.get());
    SDL_Quit();

    core::Logger::debug("SDL Context successfully cleaned up");
    m_cleaned_up = true;
}
}  // namespace contexts
}  // namespace piksy
