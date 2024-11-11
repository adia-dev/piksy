#include "frame_extraction_command.hpp"

#include <core/logger.hpp>

namespace piksy {
namespace commands {

FrameExtractionCommand::FrameExtractionCommand(const SDL_Rect& extraction_rect,
                                               std::shared_ptr<rendering::Texture2D> texture,
                                               std::vector<SDL_Rect>& out_frames, bool append)
    : m_extraction_rect(extraction_rect),
      m_texture(texture),
      m_out_frames(out_frames),
      m_append(append) {}

void FrameExtractionCommand::execute() {
    if (m_texture == nullptr) return;

    SDL_Rect texture_rect{0, 0, m_texture->width(), m_texture->height()};

    SDL_Rect intersection_rect;
    if (SDL_IntersectRect(&m_extraction_rect, &texture_rect, &intersection_rect) == SDL_FALSE) {
        return;
    }

    if (intersection_rect.w > 0 && intersection_rect.h > 0) {
        void* pixels;
        int pitch;
        auto texture = m_texture;

        if (SDL_LockTexture(texture->get(), &intersection_rect, &pixels, &pitch) < 0) {
            core::Logger::error("Failed to lock the texture: %s", SDL_GetError());
            return;
        }

        try {
            cv::Mat mat(intersection_rect.h, intersection_rect.w, CV_8UC4, pixels, pitch);

            cv::Mat mat_gray;
            cv::cvtColor(mat, mat_gray, cv::COLOR_RGBA2GRAY);

            // TODO: Make this dynamic
            int threshold_value = 1;
            cv::Mat thresholded;
            cv::threshold(mat_gray, thresholded, threshold_value, 255, cv::THRESH_BINARY);

            // TODO: Make this dynamic
            int dilation_size = 2;
            cv::Mat element = cv::getStructuringElement(
                cv::MORPH_RECT, cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1),
                cv::Point(dilation_size, dilation_size));

            cv::Mat dilated;
            cv::dilate(thresholded, dilated, element);

            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(dilated, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            if (!m_append) {
                m_out_frames.clear();
            }
            for (const auto& contour : contours) {
                cv::Rect bounding_rect = cv::boundingRect(contour);

                SDL_Rect frame_rect{bounding_rect.x + intersection_rect.x,
                                    bounding_rect.y + intersection_rect.y, bounding_rect.width,
                                    bounding_rect.height};

                m_out_frames.push_back(frame_rect);
            }
            // TODO: Sort the frames by their position, based on the selection rect
        } catch (const std::exception& ex) {
            core::Logger::error("Failed to process the selected area: %s", ex.what());
        }

        SDL_UnlockTexture(texture->get());
    }
}
}  // namespace commands
}  // namespace piksy
