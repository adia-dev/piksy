#pragma once

#include <SDL_rect.h>
#include <SDL_render.h>

#include <command/command.hpp>
#include <core/state.hpp>
#include <memory>
#include <opencv2/opencv.hpp>
#include <rendering/texture2D.hpp>
#include <vector>

#include "rendering/frame.hpp"

namespace piksy {
namespace commands {
// TODO: decouple the dependency to SDL, using a Rect struct and pass the pixels in the method
class FrameExtractionCommand : public Command {
   public:
    FrameExtractionCommand(const SDL_Rect& extraction_rect,
                           std::shared_ptr<rendering::Texture2D> texture,
                           std::vector<rendering::Frame>& out_frames, bool append = false,
                           bool preview_mode = false);

    virtual void execute() override;

   private:
    bool frames_are_equal(const rendering::Frame& a, const rendering::Frame& b, int tolerance = 2);

    void sort_frames(std::vector<rendering::Frame>& frames);

   private:
    SDL_Rect m_extraction_rect;
    std::shared_ptr<rendering::Texture2D> m_texture;
    std::vector<rendering::Frame>& m_out_frames;
    bool m_append;
    bool m_preview_mode;
};
}  // namespace commands
}  // namespace piksy
