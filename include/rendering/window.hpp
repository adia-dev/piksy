#pragma once

#include <SDL_video.h>

#include <core/config.hpp>
#include <memory>

namespace piksy {
namespace rendering {
class Window {
   public:
    ~Window();

    void init(core::WindowConfig& config);
    void cleanup();

    const SDL_Window* get() const;
    SDL_Window* mutable_get();

   private:
    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> _window{nullptr, SDL_DestroyWindow};
};
}  // namespace rendering
}  // namespace piksy
