#include <command/frame_extraction_command.hpp>
#include <core/logger.hpp>
#include <rendering/frame.hpp>

namespace piksy {
namespace commands {
FrameExtractionCommand::FrameExtractionCommand(const SDL_Rect& extraction_rect,
                                               std::shared_ptr<rendering::Texture2D> texture,
                                               std::vector<rendering::Frame>& out_frames,
                                               bool append, bool preview_mode)
    : m_extraction_rect(extraction_rect),
      m_texture(texture),
      m_out_frames(out_frames),
      m_append(append),
      m_preview_mode(preview_mode) {}

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

        if (SDL_LockTexture(m_texture->get(), &intersection_rect, &pixels, &pitch) < 0) {
            core::Logger::error("Failed to lock texture: %s", SDL_GetError());
            return;
        }

        try {
            cv::Mat mat(intersection_rect.h, intersection_rect.w, CV_8UC4, pixels, pitch);
            cv::Mat mat_gray;
            cv::cvtColor(mat, mat_gray, cv::COLOR_RGBA2GRAY);

            int threshold_value = 1;
            cv::Mat thresholded;
            cv::threshold(mat_gray, thresholded, threshold_value, 255, cv::THRESH_BINARY);

            int dilation_size = 2;
            cv::Mat element = cv::getStructuringElement(
                cv::MORPH_RECT, cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1),
                cv::Point(dilation_size, dilation_size));

            cv::Mat dilated;
            cv::dilate(thresholded, dilated, element);

            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(dilated, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            std::vector<rendering::Frame> new_frames;
            for (const auto& contour : contours) {
                cv::Rect bounding_rect = cv::boundingRect(contour);
                rendering::Frame frame(bounding_rect.x + intersection_rect.x,
                                       bounding_rect.y + intersection_rect.y, bounding_rect.width,
                                       bounding_rect.height);

                // Check for duplicates before adding
                bool is_duplicate = false;
                if (!m_preview_mode) {
                    for (const auto& existing_frame : m_out_frames) {
                        if (frames_are_equal(frame, existing_frame)) {
                            is_duplicate = true;
                            break;
                        }
                    }
                }

                if (!is_duplicate) {
                    new_frames.push_back(frame);
                }
            }

            // Sort frames using the same logic as before
            sort_frames(new_frames);

            if (m_preview_mode) {
                // In preview mode, just replace the output frames
                m_out_frames = std::move(new_frames);
            } else if (!m_append) {
                // In normal mode, either replace or append based on the append flag
                m_out_frames = std::move(new_frames);
            } else {
                // Append new frames to existing ones
                m_out_frames.insert(m_out_frames.end(), new_frames.begin(), new_frames.end());
            }

        } catch (const std::exception& ex) {
            core::Logger::error("Failed to process the selected area: %s", ex.what());
        }

        SDL_UnlockTexture(m_texture->get());
    }
}

bool FrameExtractionCommand::frames_are_equal(const rendering::Frame& a, const rendering::Frame& b,
                                              int tolerance) {
    return std::abs(a.x - b.x) <= tolerance && std::abs(a.y - b.y) <= tolerance &&
           std::abs(a.w - b.w) <= tolerance && std::abs(a.h - b.h) <= tolerance;
}

void FrameExtractionCommand::sort_frames(std::vector<rendering::Frame>& frames) {
    const int initial_tolerance = 5;
    int current_tolerance = initial_tolerance;

    for (int i = 0; i < 3; ++i) {
        std::stable_sort(frames.begin(), frames.end(),
                         [current_tolerance](const rendering::Frame& a, const rendering::Frame& b) {
                             if (std::abs(a.y - b.y) <= current_tolerance) return a.x < b.x;
                             return a.y < b.y;
                         });
        current_tolerance *= 2;
    }
}

}  // namespace commands
}  // namespace piksy
