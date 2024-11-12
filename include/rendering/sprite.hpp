#pragma once

#include <SDL_rect.h>

#include <memory>
#include <rendering/texture2D.hpp>

namespace piksy {
namespace rendering {
class Sprite {
   public:
    Sprite();
    explicit Sprite(std::shared_ptr<Texture2D> texture, const SDL_Rect &rect = {},
                    const SDL_Rect &frame_rect = {});

    void set_size(int w, int h);
    void set_position(int x, int y);
    void set_frame_rect(const SDL_Rect &frame_rect);

    void set_selected(bool selected);
    bool is_selected() const;

    void set_texture(std::shared_ptr<Texture2D> texture);
    std::shared_ptr<Texture2D> texture() const;

    int x() const;
    int y() const;
    int width() const;
    int height() const;

    const SDL_Rect &frame_rect() const;
    const SDL_Rect &rect() const;

    void move(int dx, int dy);

    void render(SDL_Renderer *renderer, float scale = 1.0f, float offset_x = 0.0f,
                int offset_y = 0.0f) const;

   private:
    std::shared_ptr<Texture2D> m_texture;
    SDL_Rect m_rect, m_frame_rect;
    bool m_selected = false;
};
}  // namespace rendering
}  // namespace piksy
