#include <rendering/window.hpp>

#include "core/logger.hpp"

namespace piksy {
namespace rendering {

Window::~Window() { cleanup(); }

void Window::init(const core::WindowConfig& config) {
    _window.reset(SDL_CreateWindow(config.title.c_str(), SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED, config.width, config.height,
                                   config.flags));
    if (_window == nullptr) {
        core::Logger::fatal("Failed to create a Window: %s", SDL_GetError());
    }

    core::Logger::debug("Window initialized successfully");
}

void Window::cleanup() {
    _window = nullptr;
    core::Logger::debug("Window cleanup up successfully");
}

const SDL_Window* Window::get() const { return _window.get(); }

SDL_Window* Window::get() { return _window.get(); }
}  // namespace rendering
}  // namespace piksy
