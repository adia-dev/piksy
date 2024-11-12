#include <command/frame_extraction_command.hpp>
#include <core/logger.hpp>
#include <rendering/frame.hpp>

namespace piksy {
namespace commands {

FrameExtractionCommand::FrameExtractionCommand(const SDL_Rect& extraction_rect,
                                               std::shared_ptr<rendering::Texture2D> texture,
                                               std::vector<rendering::Frame>& out_frames,
                                               bool append)
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

            if (!m_append) {
                m_out_frames.clear();
            }
            for (const auto& contour : contours) {
                cv::Rect bounding_rect = cv::boundingRect(contour);

                rendering::Frame frame(bounding_rect.x + intersection_rect.x,
                                       bounding_rect.y + intersection_rect.y, bounding_rect.width,
                                       bounding_rect.height);

                m_out_frames.push_back(frame);
            }

            // Perform iterative sorting
            // TODO: Make it configurable, passes and tolerance
            const int initial_tolerance = 5;
            int current_tolerance = initial_tolerance;

            for (int i = 0; i < 3; ++i) {
                std::stable_sort(
                    m_out_frames.begin(), m_out_frames.end(),
                    [current_tolerance](const rendering::Frame& a, const rendering::Frame& b) {
                        if (std::abs(a.y - b.y) <= current_tolerance) return a.x < b.x;
                        return a.y < b.y;
                    });
                current_tolerance *= 2;
            }

        } catch (const std::exception& ex) {
            core::Logger::error("Failed to process the selected area: %s", ex.what());
        }

        SDL_UnlockTexture(texture->get());
    }
}

}  // namespace commands
}  // namespace piksy
