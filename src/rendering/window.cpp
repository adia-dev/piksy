#include <iostream>
#include <rendering/window.hpp>

namespace piksy {
namespace rendering {

Window::~Window() { cleanup(); }

void Window::init(core::WindowConfig& config) {
    _window.reset(SDL_CreateWindow(config.title.c_str(), SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED, config.width, config.height,
                                   config.flags));
    if (_window == nullptr) {
        std::cerr << "Error: SDL_CreateWindow(): " << SDL_GetError() << std::endl;
        throw std::runtime_error("Error: SDL_CreateWindow()");
    }

    std::cout << "Window initialized successfully\n";
}

void Window::cleanup() {
    _window = nullptr;
    std::cout << "Window cleaned up successfully\n";
}

const SDL_Window* Window::get() const { return _window.get(); }

SDL_Window* Window::mutable_get() { return _window.get(); }
}  // namespace rendering
}  // namespace piksy
