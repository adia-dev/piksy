#pragma once

#include <SDL_render.h>

#include <core/config.hpp>
#include <memory>
#include <rendering/window.hpp>

namespace piksy {
namespace rendering {
class Renderer {
   public:
    ~Renderer();

    void init(Window& window, core::WindowConfig& config);

    void cleanup();

    const SDL_Renderer* get() const;
    SDL_Renderer* get();

   private:
    std::shared_ptr<SDL_Renderer> m_renderer{nullptr, SDL_DestroyRenderer};
};
}  // namespace rendering
}  // namespace piksy
