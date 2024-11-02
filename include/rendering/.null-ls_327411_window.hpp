#pragma once

#include <SDL_video.h>

#include <core/application.hpp>
#include <memory>

namespace piksy {
namespace rendering {
class Window {
   public:
    Window() {
        auto& config = core::Application::get().config();

        _window.reset(SDL_CreateWindow(config.window_config.title.c_str(), SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED, config.window_config.width,
                                       config.window_config.height, config.window_config.flags));
        if (_window == nullptr) {
            printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
            throw std::runtime_error("Error: SDL_CreateWindow()");
        }
    }

    ~Window() {
        std::cout << "Window cleaned up successfully\n";
    }

    SDL_Window* get() const { return _window.get(); }

   private:
    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> _window{nullptr, SDL_DestroyWindow};
};
}  // namespace rendering
}  // namespace piksy
