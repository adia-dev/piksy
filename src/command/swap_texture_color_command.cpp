#include <command/swap_texture_color_command.hpp>

namespace piksy {
namespace commands {

SwapTextureCommand::SwapTextureCommand(const SDL_Color& m_from, const SDL_Color& m_to,
                                       std::shared_ptr<rendering::Texture2D> m_texture,
                                       uint8_t threshold)
    : m_from(m_from), m_to(m_to), m_texture(m_texture), m_threshold(threshold) {}

// TODO: A cool idea would be to optionally use SIMD here
void SwapTextureCommand::execute() {
    SDL_Texture* texture = m_texture->get();
    void* pixels;
    int pitch;

    if (SDL_LockTexture(texture, nullptr, &pixels, &pitch) < 0) {
        core::Logger::error("Failed to lock the texture to swap the background color: %s",
                            SDL_GetError());
        return;
    }

    Uint32 format;
    SDL_QueryTexture(texture, &format, nullptr, nullptr, nullptr);
    SDL_PixelFormat* pixel_format = SDL_AllocFormat(format);
    Uint32* pixel_data = static_cast<Uint32*>(pixels);

    auto is_color_close = [](const SDL_Color& c1, const SDL_Color& c2, uint8_t threshold) -> bool {
        int dr = c1.r - c2.r;
        int dg = c1.g - c2.g;
        int db = c1.b - c2.b;
        int da = c1.a - c2.a;
        return (dr * dr + dg * dg + db * db + da * da) <= (threshold * threshold);
    };

    Uint32 to_u32 = SDL_MapRGBA(pixel_format, m_to.r, m_to.g, m_to.b, m_to.a);
    int num_replaced = 0;
    for (int y = 0; y < m_texture->height(); ++y) {
        for (int x = 0; x < m_texture->width(); ++x) {
            Uint32* current_pixel = pixel_data + y * (pitch / 4) + x;
            Uint8 pr, pg, pb, pa;
            SDL_GetRGBA(*current_pixel, pixel_format, &pr, &pg, &pb, &pa);

            if (is_color_close(m_from, {pr, pg, pb, pa}, m_threshold)) {
                *current_pixel = to_u32;
                ++num_replaced;
            }
        }
    }

    core::Logger::debug("Number of pixels replaced: %d", num_replaced);
    core::Logger::info("Replaced the color (%d, %d, %d, %d) with the color (%d, %d, %d, %d)",
                       m_from.r, m_from.g, m_from.b, m_from.a, m_to.r, m_to.g, m_to.b, m_to.a);

    SDL_UnlockTexture(texture);
    SDL_FreeFormat(pixel_format);
}

}  // namespace commands
}  // namespace piksy
