#include <rendering/renderer.hpp>

namespace piksy {
namespace rendering {
Renderer::~Renderer() { cleanup(); }

void Renderer::init(Window& window, core::WindowConfig& config) {
    _renderer.reset(SDL_CreateRenderer(window.mutable_get(), -1, config.renderer_flags),
                    SDL_DestroyRenderer);

    if (_renderer == nullptr) {
        SDL_Log("Error creating SDL_Renderer!");
        throw std::runtime_error("Error: SDL_CreateRenderer()");
    }

    std::cout << "Renderer initialized successfully\n";
}

void Renderer::cleanup() {
    _renderer = nullptr;
    std::cout << "Renderer cleaned up successfully\n";
}

const SDL_Renderer* Renderer::get() const { return _renderer.get(); }

SDL_Renderer* Renderer::mutable_get() { return _renderer.get(); }
}  // namespace rendering
}  // namespace piksy
