#include <rendering/sprite.hpp>
#include <stdexcept>

namespace piksy {

namespace rendering {
Sprite::Sprite() : _texture(nullptr), _rect({}), _frame_rect({}), _selected(false) {}

Sprite::Sprite(std::shared_ptr<Texture2D> texture, const SDL_Rect &rect, const SDL_Rect &frame_rect)
    : _texture(texture), _rect(rect), _frame_rect(frame_rect) {
    if (texture == nullptr) return;

    if (_frame_rect.w == 0) {
        _frame_rect.w = texture->width();
    }

    if (_rect.w == 0) {
        _rect.w = texture->width();
    }

    if (_frame_rect.h == 0) {
        _frame_rect.h = texture->height();
    }

    if (_rect.h == 0) {
        _rect.h = texture->height();
    }
}

void Sprite::set_size(int w, int h) {
    _rect.w = w;
    _rect.h = h;
}

void Sprite::set_position(int x, int y) {
    _rect.x = x;
    _rect.y = y;
}

void Sprite::set_selected(bool selected) { _selected = selected; }

bool Sprite::is_selected() const { return _selected; }

void Sprite::set_texture(std::shared_ptr<Texture2D> texture) {
    _texture = texture;

    if (texture == nullptr) return;

    _frame_rect.w = texture->width();
    _frame_rect.h = texture->height();

    _rect.w = texture->width();
    _rect.h = texture->height();
}

std::shared_ptr<Texture2D> Sprite::texture() const { return _texture; }

void Sprite::set_frame_rect(const SDL_Rect &frame_rect) { _frame_rect = frame_rect; }

int Sprite::x() const { return _rect.x; };

int Sprite::y() const { return _rect.y; };

int Sprite::width() const { return _rect.w; }

int Sprite::height() const { return _rect.h; }

const SDL_Rect &Sprite::frame_rect() const { return _frame_rect; }

const SDL_Rect &Sprite::rect() const { return _rect; }

void Sprite::move(int dx, int dy) {
    _rect.x += dx;
    _rect.y += dy;
}

void Sprite::render(SDL_Renderer *renderer, float scale, float offset_x, int offset_y) const {
    if (renderer == nullptr) {
        throw std::runtime_error("Cannot render a sprite if the renderer is null.");
    }
    if (_texture == nullptr) {
        throw std::runtime_error("Cannot render a sprite if the texture is null.");
    }

    SDL_Rect scaled_rect = {static_cast<int>((_rect.x + offset_x) * scale),
                            static_cast<int>((_rect.y + offset_y) * scale),
                            static_cast<int>(_rect.w * scale), static_cast<int>(_rect.h * scale)};

    SDL_RenderCopy(renderer, _texture->get(), &_frame_rect, &scaled_rect);

    if (_selected) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        SDL_RenderDrawRect(renderer, &scaled_rect);
    }
}
}  // namespace rendering
}  // namespace piksy
