#include <iostream>
#include <rendering/renderer.hpp>

#include "core/logger.hpp"

namespace piksy {
namespace rendering {
Renderer::~Renderer() { cleanup(); }

void Renderer::init(Window& window, core::WindowConfig& config) {
    m_renderer.reset(SDL_CreateRenderer(window.get(), -1, config.renderer_flags),
                     SDL_DestroyRenderer);

    if (m_renderer == nullptr) {
        SDL_Log("Error creating SDL_Renderer!");
        throw std::runtime_error("Error: SDL_CreateRenderer()");
    }

    if (SDL_SetRenderDrawBlendMode(m_renderer.get(), SDL_BLENDMODE_BLEND) < 0) {
        throw std::runtime_error("Failed to set SDL Renderer blend mode.");
    }

    core::Logger::debug("Renderer initialized successfully");
}

void Renderer::cleanup() {
    m_renderer = nullptr;
    core::Logger::debug("Renderer cleaned up successfully");
}

const SDL_Renderer* Renderer::get() const { return m_renderer.get(); }

SDL_Renderer* Renderer::get() { return m_renderer.get(); }
}  // namespace rendering
}  // namespace piksy
