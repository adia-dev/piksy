#pragma once

#include <SDL_video.h>

#include <core/config.hpp>
#include <memory>

namespace piksy {
namespace rendering {
class Window {
   public:
    ~Window();

    void init(const core::WindowConfig& config);
    void cleanup();

    const SDL_Window* get() const;
    SDL_Window* get();

   private:
    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> m_window{nullptr, SDL_DestroyWindow};
};
}  // namespace rendering
}  // namespace piksy
