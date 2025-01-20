#pragma once

#include <SDL.h>
#include <SDL2/SDL_ttf.h>

#include <core/config.hpp>
#include <rendering/renderer.hpp>
#include <rendering/window.hpp>

#if !SDL_VERSION_ATLEAST(2, 0, 17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

namespace piksy {
namespace contexts {
class SDLContext {
   public:
    ~SDLContext();

    void init(const core::WindowConfig& window_config);
    void cleanup(rendering::Window& window, rendering::Renderer& renderer);

   private:
    bool m_cleaned_up = false;
};

}  // namespace contexts
}  // namespace piksy
