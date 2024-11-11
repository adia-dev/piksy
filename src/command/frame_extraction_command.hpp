#pragma once

#include <SDL_rect.h>
#include <SDL_render.h>

#include <command/command.hpp>
#include <core/state.hpp>
#include <memory>
#include <opencv2/opencv.hpp>
#include <rendering/texture2D.hpp>
#include <vector>

namespace piksy {
namespace commands {
class FrameExtractionCommand : public Command {
   public:
    explicit FrameExtractionCommand(const SDL_Rect& extraction_rect,
                                    std::shared_ptr<rendering::Texture2D> texture,
                                    std::vector<SDL_Rect>& out_frames, bool append = false);

    virtual void execute() override;

   private:
    SDL_Rect m_extraction_rect;
    std::shared_ptr<rendering::Texture2D> m_texture;
    std::vector<SDL_Rect>& m_out_frames;
    bool m_append = false;
};
}  // namespace commands
}  // namespace piksy
