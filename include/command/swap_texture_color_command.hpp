#pragma once

#include <SDL_rect.h>
#include <SDL_render.h>

#include <command/command.hpp>
#include <core/state.hpp>
#include <cstdint>
#include <memory>
#include <opencv2/opencv.hpp>
#include <rendering/texture2D.hpp>

namespace piksy {
namespace commands {
class SwapTextureCommand : public Command {
   public:
    SwapTextureCommand(const SDL_Color& m_from, const SDL_Color& m_to,
                       std::shared_ptr<rendering::Texture2D> m_texture, uint8_t threshold = 1);

    virtual void execute() override;

   private:
    SDL_Color m_from, m_to;
    std::shared_ptr<rendering::Texture2D> m_texture;
    uint8_t m_threshold = 1;
};
}  // namespace commands
}  // namespace piksy
