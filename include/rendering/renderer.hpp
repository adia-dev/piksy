#pragma once

#include <SDL_render.h>

#include <iostream>
#include <memory>

#include "core/config.hpp"
#include "rendering/window.hpp"

namespace piksy {
namespace rendering {
class Renderer {
   public:
    ~Renderer();

    void init(Window& window, core::WindowConfig& config);

    void cleanup();

    const SDL_Renderer* get() const;
    SDL_Renderer* mutable_get();

   private:
    std::shared_ptr<SDL_Renderer> _renderer{nullptr, SDL_DestroyRenderer};
};
}  // namespace rendering
}  // namespace piksy
