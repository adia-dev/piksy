#pragma once

#include <SDL_rect.h>

#include <rendering/texture2D.hpp>

namespace piksy {
class Sprite {
   public:
    explicit Sprite(std::shared_ptr<Texture2D> texture, const SDL_Rect &rect = {},
                    const SDL_Rect &frame_rect = {});

    void set_size(int w, int h);
    void set_position(int x, int y);
    void set_frame_rect(const SDL_Rect &frame_rect);

    void set_selected(bool selected);
    bool is_selected() const;

    int x() const;
    int y() const;
    int width() const;
    int height() const;

    const SDL_Rect &frame_rect() const;
    const SDL_Rect &rect() const;

    void move(int dx, int dy);

    void render(SDL_Renderer *renderer) const;

   private:
    std::shared_ptr<Texture2D> _texture;
    SDL_Rect _rect, _frame_rect;
    bool _selected = false;
};
}  // namespace piksy
